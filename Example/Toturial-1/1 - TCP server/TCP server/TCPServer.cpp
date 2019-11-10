
#include "TCPServer.h"

#pragma comment( lib, "WS2_32.lib" ) // winsock2.h

#define DEFAULT_BUFF_LENGTH  512

TCPServer::TCPServer()
{
	ZeroMemory( this, sizeof( TCPServer ) ); // Initial wholle TCPServer class's memory to zero.
}

bool TCPServer::Create( WORD port )
{
	if( CreateWinsock( port ) == false )
		return false;

	CreateSucceedFlag = true;
	RunFlag = true;

	return true;
}

bool TCPServer::CreateWinsock( WORD port )
{
	WSADATA wsa_data = {0};
	int result = 0;

	// Winsock version 2.2
	result = WSAStartup( 0x202, &wsa_data );
	if( result != 0 )
	{
		printf( "WSAStartup failed, error: %d\n", result );
		return false;
	}

	// Create a local listen socket, to accept client's connection
	// AF_INET : IPv4 Internet Protocol
	// SOCK_STREAM : Data streaming, corresponding to TCP transmit protocal
	// IPPROTO_TCP : Use TCP transmit Protocal
	ListenSocket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if( ListenSocket == INVALID_SOCKET )
	{
		printf( "socket failed, error: %ld\n", WSAGetLastError() );
		return false;
	}

	// Setting Address Info
	sockaddr_in addr = {0};
	// Use IPv4 Internet Protocol
	addr.sin_family = AF_INET;
	// Set port, prepare a port to listen
	//( use htons() function to transform is because Network Byte Order may be different to Host Byte Order
	addr.sin_port = htons( port );
	// Set address to 0.0.0.0, means it can receive all package from binding port
	// regardless Network Cark address
	addr.sin_addr.s_addr = INADDR_ANY;

	// Bind to ListenSocket, if binding fail, reason could be port occupied.
	result = bind( ListenSocket, (SOCKADDR *)&addr, sizeof( addr ) );
	if( result == SOCKET_ERROR )
	{
		printf( "bind failed, error: %d, port= %d\n", WSAGetLastError(), port );
		return false;
	}


	// Enable listen, could receive connection from client
	// Second parm is use to set maximum listen list, SOMAXCONN means system maximum
	result = listen( ListenSocket, SOMAXCONN );
	if( result == SOCKET_ERROR )
	{
		printf( "listen failed, error: %d\n", WSAGetLastError() );
		return false;
	}

	return true;
}

bool TCPServer::Accept()
{
	printf( "TCP server - Listen to wait...\n" );

	// Accept a client socket's connection, waitint in accept function until it successful build connection with client
	Client.Socket = accept( ListenSocket, NULL, NULL );
	if( Client.Socket == INVALID_SOCKET )
	{
		printf( "accept failed, error: %d\n", WSAGetLastError() );
		return false;
	}

	printf( "accept ok.\n" );

	return true;
}

void TCPServer::Receive()
{
	char recv_buff[ DEFAULT_BUFF_LENGTH ] = {0};
	int  recv_buff_len = DEFAULT_BUFF_LENGTH;
	int  recv_len = 0;
	int  send_result = 0;
	int  error = 0;

	while( RunFlag )
	{
		printf( "\n" );

		recv_len = recv( Client.Socket, recv_buff, recv_buff_len, 0 ); // Receive package, blocking
		if( recv_len > 0 )
		{
			printf( "bytes recv_len= %d, recv_buff= %s\n", recv_len, recv_buff );

			// Echo - Return recv_buff to client
			send_result = send( Client.Socket, recv_buff, recv_len, 0 );
			if( send_result == SOCKET_ERROR )
			{
				printf( "send failed, error: %d\n", WSAGetLastError() );
				break;
			}
			printf( "bytes sent: %d\n", send_result );
			memset( recv_buff, 0, DEFAULT_BUFF_LENGTH );
		}
		else if( recv_len == 0 )
		{
			printf( "client - connection closed.\n" );
			break;
		}
		else
		{
			error = WSAGetLastError();
			printf( "recv failed, error: %d\n", error );
			break;
		}
	};
}

void TCPServer::CloseSocket( SOCKET &socket )
{
	if( socket != INVALID_SOCKET )
	{
		closesocket( socket );
		socket = 0;
	}
}

void TCPServer::Release()
{
	if( Client.Socket != INVALID_SOCKET )
	{
		// Shutdown listen socket connect, normally will delay for a while after shutdown
		// Make sure all pocket are transmited, then close it.
		// SD_SEND：Shutdown Send function.
		int result = shutdown( Client.Socket, SD_SEND );
		if( result == SOCKET_ERROR )
		{
			printf( "shutdown failed, error: %d\n", WSAGetLastError() );
		}
		CloseSocket( Client.Socket );
	}

	CloseSocket( ListenSocket );

	WSACleanup(); // Release winsock
}

void TCPServer::Run()
{
	if( CreateSucceedFlag )
	{
		if( Accept() )
			Receive();
	}
}