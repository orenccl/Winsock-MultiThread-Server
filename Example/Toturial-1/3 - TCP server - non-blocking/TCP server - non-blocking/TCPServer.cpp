#include "TCPServer.h"

#pragma comment( lib, "WS2_32.lib" ) // winsock2.h

#define DEFAULT_BUFF_LENGTH  512

TCPServer::TCPServer()
{
	ZeroMemory( this, sizeof( TCPServer ) );
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

	result = WSAStartup( 0x202, &wsa_data );
	if( result != 0 )
	{
		printf( "WSAStartup failed, error: %d\n", result );
		return false;
	}

	ListenSocket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if( ListenSocket == INVALID_SOCKET )
	{
		printf( "socket failed, error: %ld\n", WSAGetLastError() );
		return false;
	}

	sockaddr_in addr = {0};
	addr.sin_family = AF_INET;         
	addr.sin_port = htons( port );     
	addr.sin_addr.s_addr = INADDR_ANY; 

	result = bind( ListenSocket, (SOCKADDR *)&addr, sizeof( addr ) );
	if( result == SOCKET_ERROR )
	{
		printf( "bind failed, error: %d, port= %d\n", WSAGetLastError(), port );
		return false;
	}


	result = listen( ListenSocket, SOMAXCONN );
	if( result == SOCKET_ERROR )
	{
		printf( "listen failed, error: %d\n", WSAGetLastError() );
		return false;
	}

	// Set ListenSocket to non-blocking mode
	// FIONBIO : set/clear non-blocking i/o
	unsigned long non_blocking_mode = 1; // ­1 = non-blocking
	result = ioctlsocket( ListenSocket, FIONBIO, &non_blocking_mode );
	if( result == SOCKET_ERROR )
	{
		printf( "ioctlsocket failed, error: %d\n", WSAGetLastError() );
		return false;
	}

	return true;
}

bool TCPServer::Accept()
{
	printf( "TCP server - Listen to wait...\n" );

	int error = 0;

	// Will not be block
	while( RunFlag )
	{
		Client.Socket = accept( ListenSocket, NULL, NULL );
		if( Client.Socket == INVALID_SOCKET )
		{
			error = WSAGetLastError();
			if( error == WSAEWOULDBLOCK )
			{
				printf( "." );
				Sleep( 100 );
				continue;
			}
			printf( "accept failed, error: %d\n", error );
		}
		break;
	}

	printf( "\n" );
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
		recv_len = recv( Client.Socket, recv_buff, recv_buff_len, 0 );
		if( recv_len > 0 )
		{
			printf( "\n bytes recv_len= %d, recv_buff= %s\n", recv_len, recv_buff );

			send_result = send( Client.Socket, recv_buff, recv_len, 0 );
			if( send_result == SOCKET_ERROR )
			{
				printf( "\n send failed, error: %d\n", WSAGetLastError() );
				break;
			}
			printf( " bytes sent: %d\n", send_result );
			memset( recv_buff, 0, DEFAULT_BUFF_LENGTH );
		}
		else if( recv_len == 0 )
		{
			printf( "\n client - connection closed.\n" );
			break;
		}
		else
		{
			error = WSAGetLastError();
			if( error == WSAEWOULDBLOCK )
			{
				::Sleep( 100 );
				printf( "." );
				continue;
			}	
			printf( "\n recv failed, error: %d\n", error );
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
		int result = shutdown( Client.Socket, SD_SEND );
		if( result == SOCKET_ERROR )
		{
			printf( "shutdown failed, error: %d\n", WSAGetLastError() );
		}
		CloseSocket( Client.Socket );
	}

	CloseSocket( ListenSocket );

	WSACleanup();
}

void TCPServer::Run()
{
	if( CreateSucceedFlag )
	{
		if( Accept() )
			Receive();
	}
}