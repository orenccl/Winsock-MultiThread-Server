#include "TCPServer.h"
#include <ws2tcpip.h>
#include <stdio.h>

#pragma comment( lib, "WS2_32.lib" ) // winsock2.h

#define DEFAULT_BUFF_LENGTH  512

TCPServer::TCPServer()
{
	memset( this, 0, sizeof( TCPServer ) );
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

	return true;
}

void TCPServer::Select()
{
	// Initialize socket array
	fd_set fds_read = {0};

	const long one_millisecond = 1000;
	const long milliseconds = one_millisecond * 100;
	timeval timeout = { 0, milliseconds }; // 0 = ms
	int result = 0;

	printf( "TCP server - select mode\n" );

	while( RunFlag )
	{
		// Clear socket array
		FD_ZERO( &fds_read );
  
		// Add ListenSocket to socket array
		FD_SET( ListenSocket, &fds_read );

		// Add Client Socket to socket array
		if( Client.Socket > 0 )
			FD_SET( Client.Socket, &fds_read );

		printf( "." );

		// select will wait until recv event occur
		// timeout time is the period program return from select function
		result = select( 0, &fds_read, NULL, NULL, &timeout );
		if( result == SOCKET_ERROR )
		{
			RunFlag = false;
			printf( "select failed, error: %d\n" , WSAGetLastError() );
			break;
		}

		if( FD_ISSET( ListenSocket, &fds_read ) ) // Check if is new connection
		{
			Accept();
		}

		if( Client.Socket > 0 )
		{
			if( FD_ISSET( Client.Socket, &fds_read ) ) // Check if is new pocket
			{
				if( Receive() == false )
				{
					CloseSocket( Client.Socket );
				}
			}
		}
	}
}

void TCPServer::Accept()
{
	Client.Socket = accept( ListenSocket, NULL, NULL );
	if( Client.Socket == INVALID_SOCKET )
	{
		printf( "\n accept failed, error: %d\n", WSAGetLastError() );
		return;
	}
	printf( "\n accept ok, create a new connection, socket= %d\n", Client.Socket );
}

bool TCPServer::Receive()
{
	char recv_buff[ DEFAULT_BUFF_LENGTH ] = {0};
	int  recv_buff_len = DEFAULT_BUFF_LENGTH;

	int recv_len = recv( Client.Socket, recv_buff, recv_buff_len, 0 );
	if( recv_len > 0 )
	{
		printf( "\n bytes recv_len= %d, recv_buff= %s\n", recv_len, recv_buff );

		int send_result = send( Client.Socket, recv_buff, recv_len, 0 );
		if( send_result == SOCKET_ERROR )
		{
			printf( "\n send failed, socket= %d, error=%d\n", Client.Socket, WSAGetLastError() );
			return false;
		}
		printf( " bytes sent: %d\n", send_result );
		return true;
	}
	else if( recv_len == 0 )
	{
		printf( "\n connection closed, socket= %d\n", Client.Socket );
	}
	else
	{
		int error = WSAGetLastError();
		printf( "\n recv failed, socket= %d, error= %d\n", Client.Socket, error );
	}
	return false;
}


void TCPServer::CloseSocket( SOCKET &socket )
{
	if( socket != INVALID_SOCKET )
	{
		closesocket( socket );
		socket = 0;
	}
}


void TCPServer::ShutdownAndCloseSocket()
{
	if( Client.Socket != INVALID_SOCKET )
	{
		int result = shutdown( Client.Socket, SD_SEND );
		if( result == SOCKET_ERROR )
		{
			printf( "shutdown failed, error: %d\n", WSAGetLastError() );
		}
		closesocket( Client.Socket );
	}
}

void TCPServer::Release()
{
	RunFlag = false;
	CloseSocket( ListenSocket );
	
	ShutdownAndCloseSocket();

	WSACleanup();
}

void TCPServer::Run()
{
	if( CreateSucceedFlag )
	{
		Select();
	}
}