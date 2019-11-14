#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdio.h>
#include <conio.h>

#pragma comment ( lib, "Ws2_32.lib" ) // winsock2.h

#define DEFAULT_BUFF_LENGTH  512
#define SERVER_PORT          27015

SOCKET ConnectSocket = INVALID_SOCKET;

void TCPClientCreate()
{
	printf( "TCP client create\n" );

	WSADATA wsaData = {0};
	int result = 0;

	result = WSAStartup( 0x202, &wsaData );
	if( result != 0 )
	{
		printf( "WSAStartup failed, error: %d\n", result );
		return;
	}

	ConnectSocket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if( ConnectSocket == INVALID_SOCKET )
	{
		printf( "socket failed, error: %ld\n", WSAGetLastError() );
		return;
	}

	const char *ip = "127.0.0.1";            
	sockaddr_in addr = {0};
	addr.sin_family  = AF_INET;              
	addr.sin_port    = htons( SERVER_PORT ); 
	inet_pton( AF_INET, ip, &addr.sin_addr );

	result = connect( ConnectSocket, (SOCKADDR *)&addr, sizeof( addr ) );
	if( result == SOCKET_ERROR )
	{
		printf( "connect failed, error: %d\n", WSAGetLastError() );
		return;
	}

	if( ConnectSocket == INVALID_SOCKET )
	{
		printf( "unable to connect to server !\n" );
		return;
	}
	else
		printf( "connect to server !\n" );


	unsigned long non_blocking_mode = 1;
	result = ioctlsocket( ConnectSocket, FIONBIO, &non_blocking_mode );
	if( result == SOCKET_ERROR )
	{
		printf( "ioctlsocket failed, error: %d", WSAGetLastError() );
		return;
	}


	int  keyboard_state = 0, ch = 0;
	int  sendbuf_use_index = 0, send_length = 0;
	char sendbuf[ DEFAULT_BUFF_LENGTH ] = {0};
	char recvbuf[ DEFAULT_BUFF_LENGTH ] = {0};
	int  recvbuflen = DEFAULT_BUFF_LENGTH;
	
	#define BACKSPCAE_KEY_CODE  8
	#define ENTER_KEY_CODE     13

	printf( "chat input... \n" );

	// Use winsock non-blocking, it will not block in fget or recv
	while( 1 )
	{
		keyboard_state = _kbhit(); // Key Board Hit
	
		if( keyboard_state > 0 )
		{
			ch = _getch(); // Get char input
			printf( "%c", ch );
			if( ch == ENTER_KEY_CODE )
			{
				printf( "\n" );
				if( strcmp( sendbuf, "exit" ) == 0 )
					break;

				send_length = strlen( sendbuf );
				if( send_length > 0 )
				{
					result = send( ConnectSocket, sendbuf, send_length, 0 );
					if( result == SOCKET_ERROR )
					{
						printf( "send failed, error: %d\n", WSAGetLastError() );
						return;
					}
					printf( "bytes sent: %ld\n", result );

					memset( sendbuf, 0, DEFAULT_BUFF_LENGTH );
					sendbuf_use_index = 0;
				}
			}
			else if( ch == BACKSPCAE_KEY_CODE )
			{
				if( sendbuf_use_index - 1 >= 0 )
					sendbuf[ --sendbuf_use_index ] = 0;
			}
			else
			{
				sendbuf[ sendbuf_use_index++ ] = ch;
			}
		}


		result = recv( ConnectSocket, recvbuf, recvbuflen, 0 );
		if( result > 0 )
		{
			printf( "bytes recv_len= %d, recv_buff= %s\n", result, recvbuf );
			memset( recvbuf, 0, DEFAULT_BUFF_LENGTH );
		}
		else if( result == 0 )
		{
			printf( "server connection closed.\n" );
			break;
		}
		else
		{
			int error = WSAGetLastError();
			if( error == WSAEWOULDBLOCK ) // 10035 no pocket
			{
			}
			else
			{
				printf( "recv failed, error: %d", error );
				break;
			}
		}


		Sleep( 1 ); // 1ms
	}

	// Shutdown send function
	result = shutdown( ConnectSocket, SD_SEND );
	if( result == SOCKET_ERROR )
	{
		printf( "shutdown failed, error: %d\n", WSAGetLastError() );
		return;
	}

	Sleep( 100 ); // 100ms, wait client shutdown properly.

	do
	{
		result = recv( ConnectSocket, recvbuf, recvbuflen, 0 );
		if( result > 0 )
			printf( "final bytes received: %d\n", result );
		else if( result == 0 )
			printf( "client connection closed.\n" );
		else
		{
			int error = WSAGetLastError();
			if( error == WSAEWOULDBLOCK ) // 10035 no pocket
			{
				printf( "client connection closed. - WSAEWOULDBLOCK\n" );
			}
			else
			{
				printf( "final recv failed, error: %d", error );
				break;
			}
		}

	} while( result > 0 );
}

void TCPClientRelease()
{
	if( ConnectSocket != INVALID_SOCKET )
		closesocket( ConnectSocket );

	WSACleanup();
}

int main()
{
	TCPClientCreate();
	TCPClientRelease();

	return 0;
}