using UnityEngine;
using System;
using System.IO;
using System.Net.Sockets;
using System.Threading;

public class HeadPacketInfo
{
	public ushort PacketLength;
	public ushort PacketType; // _PACKET_TYPE_
	public ushort PacketOrderNumber;
};

public class PacketBuffer : HeadPacketInfo
{
	public byte[] Buffer = new byte[ 512 ];
};

enum _PACKET_TYPE_
{
	PACKET__C2S_ALIVE = 1,

	PACKET__C2S_NAME,
	PACKET__C2S_MESSAGE,
	PACKET__S2C_MESSAGE,
    PACKET__C2S_MESSAGE_2,
    PACKET__S2C_MESSAGE_2,
};

public struct SocketClient
{
	public Socket   Socket;
	public string   Name;
	public ushort   S2C_PacketOrderNumber;
	public ushort   C2S_PacketOrderNumber;
};

public class TCPClient : MonoBehaviour
{
	AsyncCallback RequestCallback = new AsyncCallback( OnAsyncFinishConnect );
	TcpClient     TheSocket;
	NetworkStream TheStream;
	BinaryWriter  TheWriter;
	BinaryReader  TheReader;

	public string ServerIP   = "127.0.0.1"; // "localhost";
	public int    ServerPort = 27015;

	const int     HeadPacketInfo_Length = 6; // HeadPacketInfo size
	int           RecvPacketLength = PACKET_LENGTH_SIZE;
	int           RecvPacketBufferIndex;

	const int     PACKET_LENGTH_SIZE = 2;   // ushort size
	const int     PacketBufferMax = 1024;
	byte[]        RecvPacketBuffer = new byte[ PacketBufferMax ];

    byte[]        SendPacketBuffer = new byte[ PacketBufferMax ];
    int           SendPacketBufferUseLength;

    SocketClient  Client;

	Thread        AliveThread; // HeartbeatThread
	bool          AliveThreadProcRunFlag = true;
	int           AlivePacketTimer;

    void Start()
	{
        // Create a HeartbeatThread, avoid lost Hearthbeat
        AliveThread = new Thread( OnAliveThreadProc );
		AliveThread.IsBackground = true; // true = BackThread whatever it's job done or not, stop directly when receive command.
		AliveThread.Start();

		InvokeRepeating( "OnReadPacket", 0.1f, 0.1f ); // Read binary package every period.

		ConnectToServer( ServerIP, ServerPort );                  // client connect to server test.
		InvokeRepeating( "OnTest_Send_C2S_Message", 1.5f, 1.5f ); // Messenge test
    }

	public void ConnectToServer( string ip, int port )
	{
		ServerIP = ip;
        ServerPort = port;
		
		Debug.Log( "ConnectToServer   TcpClient connect START" );

		try
		{
			TheSocket = new TcpClient();
			TheSocket.BeginConnect( ServerIP, ServerPort, RequestCallback, this );
		}
		catch( SocketException e )
		{
			Debug.LogError( "ConnectToServer   SocketException ErrorCode= " + e.ErrorCode + ", error= " + e );
		}
	}

    static void OnAsyncFinishConnect( IAsyncResult result )
	{
		TCPClient _this = (TCPClient)result.AsyncState;
		_this.ConnectToServerFinish( result );
    }

	void ConnectToServerFinish( IAsyncResult result )
	{
		try
		{
			TheSocket.EndConnect( result );
			TheSocket.NoDelay = true; // Trun off TCP_NODELAY Nagle's Algorithm(Speed up)

			TheStream = TheSocket.GetStream();
			TheWriter = new BinaryWriter( TheStream );
			TheReader = new BinaryReader( TheStream );
		
			Debug.Log( "ConnectToServerFinish   TcpClient connect FINISHED" );
		}
		catch( SocketException e )
		{
			Debug.LogError( "ConnectToServerFinish   SocketException ErrorCode= " + e.ErrorCode + ", error= " + e );
		}
	}

    void OnAliveThreadProc()
	{
		while( AliveThreadProcRunFlag )
		{
			if( TheSocket != null && TheSocket.Connected )
			{
				if( ++AlivePacketTimer >= 30 ) // If no messenge more than 30s.
				{
					Send_C2S_Alive(); // Send Heartbeat
					Debug.Log( "OnAliveThreadProc   Send_C2S_Alive" );
				}
			}
			Thread.Sleep( 1000 ); // Wait 1s.
		}
	}

	void MakeSendPacketBuffer( ushort value )
	{
		byte[] value_bytes = BitConverter.GetBytes( value );
		int string_max = value_bytes.Length;

		for( int i = 0; i < string_max; ++i )
		{
			if( SendPacketBufferUseLength >= PacketBufferMax )
			{
				Debug.LogError( "MakeSendPacketBuffer   error : if( SendPacketBufferUseLength >= PacketBufferMax ), string_max= " + string_max );
				break;
			}
			SendPacketBuffer[ SendPacketBufferUseLength++ ] = value_bytes[ i ];
		}
	}

    void MakeSendPacketBuffer( string str )
	{
		byte[] str_bytes = System.Text.Encoding.Default.GetBytes( str );
		int string_max = str_bytes.Length;

		for( int i = 0; i < string_max; ++i )
		{
			if( SendPacketBufferUseLength >= PacketBufferMax )
			{
				Debug.LogError( "MakeSendPacketBuffer   error : if( SendPacketBufferUseLength >= PacketBufferMax ), string_max= " + string_max );
				break;
			}
			SendPacketBuffer[ SendPacketBufferUseLength++ ] = str_bytes[ i ];
		}
	}
   
	public void SendPacket( byte[] packet, int packet_length )
	{
		SendPacketBufferUseLength = 0;

		if( TheSocket.Connected == false || TheWriter == null )
			return;

		try
		{
			TheWriter.Write( packet, 0, packet_length );
			TheWriter.Flush();
		}
		catch( Exception e )
		{
			Debug.LogError( "SendPacket   Exception error= " + e );
			return;
		}

		AlivePacketTimer = 0;

		Debug.Log( "SendPacket   send_length= " + packet_length );
	}

	bool ReadPacket_ContinueReadFlag = false;
	void OnReadPacket()
	{
		if( TheStream == null || TheStream.DataAvailable == false )
			return;
		if( TheReader == null )
			return;

        // Distributed read package
        // No blocking in a while loop, avoid try many times in same loop.
        // If can't read all contene at a time.
        // Read at next action.
        if ( ReadPacket_ContinueReadFlag == false )
		{
			ReadPacket_Length();
			ReadPacket_Content();
		}
		else if( ReadPacket_ContinueReadFlag == true )
		{
			ReadPacket_Content();
		}
	}

	void ReadPacket_Length()
	{
		int recv_length = 0;

		while( RecvPacketLength > 0 )
		{
			try
			{
				// Read package length( 2 bytes )
				recv_length = TheReader.Read( RecvPacketBuffer, RecvPacketBufferIndex, RecvPacketLength );
			}
			catch( Exception e )
			{
				Debug.LogError( "ReadPacket_Length   Exception error= " + e );
				return;
			}

			if( recv_length > 0 )
			{
				RecvPacketLength -= recv_length;
				RecvPacketBufferIndex += recv_length;
			}
			else
			{
				Debug.LogError( "ReadPacket_Length   ERROR recv_length= " + recv_length );
				return;
			}
		}

		RecvPacketLength = BitConverter.ToInt16( RecvPacketBuffer, 0 );
		RecvPacketLength -= PACKET_LENGTH_SIZE;

		ReadPacket_ContinueReadFlag = true;
	}

	void ReadPacket_Content()
	{
		int recv_length = 0;

		while( RecvPacketLength > 0 )
		{
			try
			{
				// Read a completely package
				recv_length = TheReader.Read( RecvPacketBuffer, RecvPacketBufferIndex, RecvPacketLength );
			}
			catch( Exception e )
			{
				Debug.LogError( "ReadPacket_Content   Exception error= " + e );
				return;
			}

			if( recv_length > 0 )
			{
				RecvPacketLength      -= recv_length;
				RecvPacketBufferIndex += recv_length;
			}
			else
			{
				Debug.LogError( "ReadPacket_Content   ERROR recv_length= " + recv_length );
				return;
			}
		}

		ReadPacket_ContinueReadFlag = false;
		
		Debug.Log( "ReadPacket_Content   total_recv_length= " + RecvPacketBufferIndex );

		// Transfer to package content
		HeadPacketInfo packet = new HeadPacketInfo();
		packet.PacketLength      = (ushort)BitConverter.ToInt16( RecvPacketBuffer, 0 );
		packet.PacketType        = (ushort)BitConverter.ToInt16( RecvPacketBuffer, 2 );
		packet.PacketOrderNumber = (ushort)BitConverter.ToInt16( RecvPacketBuffer, 4 );
		string buffer = System.Text.Encoding.Default.GetString( RecvPacketBuffer, HeadPacketInfo_Length, RecvPacketBuffer.Length - HeadPacketInfo_Length );

		Debug.Log( "ReadPacket_Content   packet.PacketLength= " + packet.PacketLength );
		Debug.Log( "ReadPacket_Content   packet.PacketType= " + packet.PacketType );
		Debug.Log( "ReadPacket_Content   packet.PacketOrderNumber= " + packet.PacketOrderNumber );
		Debug.Log( "ReadPacket_Content   packet.Buffer= " + buffer );

		if( packet.PacketOrderNumber == ++Client.S2C_PacketOrderNumber )
			Debug.Log( "ReadPacket_Content   OK" );
		else
			Debug.Log( "ReadPacket_Content   ERROR PacketOrderNumber : packet.PacketOrderNumber= " + packet.PacketOrderNumber + ", Client.S2C_PacketOrderNumber= " + Client.S2C_PacketOrderNumber );

		Array.Clear( RecvPacketBuffer, 0, PacketBufferMax );
		RecvPacketLength = PACKET_LENGTH_SIZE;
		RecvPacketBufferIndex = 0;
	}
    
    public void CloseSocket()
	{
		AliveThreadProcRunFlag = false;

		if( TheWriter != null )
			TheWriter.Close();

		if( TheReader != null )
			TheReader.Close();

		if( TheSocket != null )
			TheSocket.Close();
	}
	
    void OnApplicationQuit()
    {
		CloseSocket();
    }
	
	void Send_C2S_Alive()
    {
		HeadPacketInfo packet    = new HeadPacketInfo();
		packet.PacketLength      = (ushort)(HeadPacketInfo_Length);
		packet.PacketType        = (ushort)_PACKET_TYPE_.PACKET__C2S_ALIVE;
		packet.PacketOrderNumber = ++Client.C2S_PacketOrderNumber;
        
		MakeSendPacketBuffer( packet.PacketLength );
		MakeSendPacketBuffer( packet.PacketType );
		MakeSendPacketBuffer( packet.PacketOrderNumber );

		SendPacket( SendPacketBuffer, SendPacketBufferUseLength );
    }

    void Send_C2S_Name( string name )
    {
		HeadPacketInfo packet    = new HeadPacketInfo();
		packet.PacketLength      = (ushort)(HeadPacketInfo_Length + name.Length);
		packet.PacketType        = (ushort)_PACKET_TYPE_.PACKET__C2S_NAME;
		packet.PacketOrderNumber = ++Client.C2S_PacketOrderNumber;
        
		MakeSendPacketBuffer( packet.PacketLength );
		MakeSendPacketBuffer( packet.PacketType );
		MakeSendPacketBuffer( packet.PacketOrderNumber );
		MakeSendPacketBuffer( name );

		SendPacket( SendPacketBuffer, SendPacketBufferUseLength );
    }

    void Send_C2S_Message( string message )
    {
		HeadPacketInfo packet    = new HeadPacketInfo();
		packet.PacketLength      = (ushort)(HeadPacketInfo_Length + message.Length);
		packet.PacketType        = (ushort)_PACKET_TYPE_.PACKET__C2S_MESSAGE;
		packet.PacketOrderNumber = ++Client.C2S_PacketOrderNumber;
        
		MakeSendPacketBuffer( packet.PacketLength );
		MakeSendPacketBuffer( packet.PacketType );
		MakeSendPacketBuffer( packet.PacketOrderNumber );
		MakeSendPacketBuffer( message );

		SendPacket( SendPacketBuffer, SendPacketBufferUseLength );
    }

    void Send_C2S_Message_2(string message)
    {
        HeadPacketInfo packet = new HeadPacketInfo();
        packet.PacketLength = (ushort)(HeadPacketInfo_Length + message.Length);
        packet.PacketType = (ushort)_PACKET_TYPE_.PACKET__C2S_MESSAGE_2;
        packet.PacketOrderNumber = ++Client.C2S_PacketOrderNumber;

        MakeSendPacketBuffer(packet.PacketLength);
        MakeSendPacketBuffer(packet.PacketType);
        MakeSendPacketBuffer(packet.PacketOrderNumber);
        MakeSendPacketBuffer(message);

        SendPacket(SendPacketBuffer, SendPacketBufferUseLength);
    }
   
    bool FirstNamePacketFlag;
	void OnTest_Send_C2S_Message()   // TEST
	{
		if( FirstNamePacketFlag == false )
		{
			FirstNamePacketFlag = true;
			Client.Name = "andy";
			Send_C2S_Name( Client.Name );
		}
		Send_C2S_Message( "hello" );
        Send_C2S_Message_2("HAHA!");
    }
}