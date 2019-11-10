#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdio.h>

#pragma comment ( lib, "Ws2_32.lib" ) // winsock2.h

#define DEFAULT_BUFF_LENGTH  512
#define SERVER_PORT          27015

void TCPClient()
{
	// This TCP Client is a example version.

	printf( "TCP client\n" );

	WSADATA wsaData = {0};
	SOCKET ConnectSocket = INVALID_SOCKET;
	int result = 0;

	// Winsock version 2.2
	result = WSAStartup( 0x202, &wsaData );
	if( result != 0 )
	{
		printf( "WSAStartup failed, error: %d\n", result );
		return;
	}

	// Create a local connect socket, to connect with server.
	// AF_INET : IPv4 Internet Protocol
	// SOCK_STREAM : Data streaming, corresponding to TCP transmit protocal
	// IPPROTO_TCP : Use TCP transmit Protocal
	ConnectSocket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if( ConnectSocket == INVALID_SOCKET )
	{
		printf( "socket failed, error: %ld\n", WSAGetLastError() );
		return;
	}

	const char *ip = "127.0.0.1";             // Connect to local IP
	sockaddr_in addr = {0};
	addr.sin_family = AF_INET;                // IPv4 Internet protocal
	addr.sin_port   = htons( SERVER_PORT );   // Set port
	inet_pton( AF_INET, ip, &addr.sin_addr ); // Set IP for client to connect to.

	result = connect( ConnectSocket, (SOCKADDR *)&addr, sizeof( addr ) ); // connect to server
	if( result == SOCKET_ERROR )
	{
		printf( "connect failed with error: %d\n", WSAGetLastError() );
		WSACleanup(); // Release winsock
		return;
	}

	if( ConnectSocket == INVALID_SOCKET )
	{
		printf( "unable to connect to server !\n" );
		WSACleanup(); // Release winsock
		return;
	}
	else
		printf( "connect to server !\n" );

	char sendbuf[ DEFAULT_BUFF_LENGTH ] = {0};
	char recvbuf[ DEFAULT_BUFF_LENGTH ] = {0};
	int recvbuflen = DEFAULT_BUFF_LENGTH;
	
	// Winsock default blocking mode.
	while( 1 )
	{
		printf( "chat input : " );
		fgets( sendbuf, sizeof( sendbuf ), stdin ); // fgets() wait for input
		if( sendbuf[0] == '\n' )
			continue;

		int send_length = strlen( sendbuf );
		if( send_length > 0 )
		{
			sendbuf[ send_length - 1 ] = 0; // When use fget(), the last one character will be '\n'.
			send_length -= 1;
		}

		if( strcmp( sendbuf, "exit" ) == 0 )
			break;

		result = send( ConnectSocket, sendbuf, send_length, 0 );
		if( result == SOCKET_ERROR )
		{
			printf( "send failed, error: %d\n", WSAGetLastError() );
			closesocket( ConnectSocket );
			WSACleanup();
			return;
		}

		printf( "bytes Sent: %ld\n", result );

		result = recv( ConnectSocket, recvbuf, recvbuflen, 0 ); // recv blocking...
		if( result > 0 )
		{
			printf( "bytes recv_len= %d, recv_buff= %s\n", result, recvbuf );
		}
		memset( recvbuf, 0, DEFAULT_BUFF_LENGTH );
	}

	result = shutdown( ConnectSocket, SD_SEND );
	if( result == SOCKET_ERROR )
	{
		printf( "shutdown failed, error: %d\n", WSAGetLastError() );
		closesocket( ConnectSocket );
		WSACleanup();
		return;
	}

	// result will be 0 when finish!
	do
	{
		result = recv( ConnectSocket, recvbuf, recvbuflen, 0 );
		if( result > 0 )
			printf( "bytes received: %d\n", result );
		else if( result == 0 )
			printf( "Connection closed\n" );
		else
			printf( "recv failed, error: %d\n", WSAGetLastError() );

	} while( result > 0 );

	closesocket( ConnectSocket );
	WSACleanup();
}

int main()
{
	TCPClient();
	return 0;
}