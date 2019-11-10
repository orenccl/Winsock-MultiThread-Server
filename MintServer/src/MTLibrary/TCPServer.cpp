#include "mtpch.h"
#include "MTLibrary/TCPServer.h"
#include "MTLibrary/MemoryPool.h"

#pragma comment( lib, "WS2_32.lib" ) // winsock2.h

#define DEFAULT_BUFF_LENGTH  512

TCPServer::TCPServer()
{
	memset(this, 0, sizeof(TCPServer));
}


bool TCPServer::Create(WORD port)
{
	const DWORD memory_pool_bytes_max = 100*MB;
	MainMemoryPool::sMemoryPoolCreate(memory_pool_bytes_max);

	if (CreateWinsock(port) == false)
		return false;

	ClientList.Create( FD_SETSIZE );

	CreateSucceedFlag = true;
	RunFlag = true;

	return true;
}


bool TCPServer::CreateWinsock(WORD port)
{
	WSADATA wsa_data = { 0 };
	int result = 0;

	result = WSAStartup( 0x202, &wsa_data);
	if (result != 0)
	{
		printf("WSAStartup failed, error: %d\n", result);
		return false;
	}

	ListenSocket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if (ListenSocket == INVALID_SOCKET)
	{
		printf( "socket failed, error: %ld\n", WSAGetLastError() );
		return false;
	}

	sockaddr_in addr = { 0 };
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;

	result = bind(ListenSocket, (SOCKADDR*)&addr, sizeof(addr));
	if (result == SOCKET_ERROR)
	{
		printf("bind failed, error: %d, port= %d\n", WSAGetLastError(), port);
		return false;
	}

	result = listen(ListenSocket, SOMAXCONN);
	if (result == SOCKET_ERROR)
	{
		printf("listen failed, error: %d\n", WSAGetLastError());
		return false;
	}

	return true;
}

void TCPServer::Select()
{
	// Initialize socket array
	fd_set readfds = { 0 };

	timeval timeout = { 0, 100 * MILLI_SECOND };
	int result = 0;

	TDNode< SocketContext >* node = NULL, * del_node = NULL;

	Logger::Log("TCP server - select mode - multi-client");

	while (RunFlag)
	{
		// Clear socket array
		FD_ZERO(&readfds);

		// Add ListenSocket into array
		FD_SET(ListenSocket, &readfds);

		// Add Clientsocket into array
		if (ClientList.NodeUseCount > 0)
		{
			node = ClientList.pHeadNode;
			while (node)
			{
				FD_SET(node->Socket, &readfds);
				node = node->pNext;
			}
		}

		printf(".");

		// select will block until new event
		result = select(0, &readfds, NULL, NULL, &timeout);
		if (result == SOCKET_ERROR)
		{
			RunFlag = false;
			Logger::Log("select failed, error: %d", WSAGetLastError());
			break;
		}

		if (FD_ISSET(ListenSocket, &readfds)) // If there has new connection
		{
			Accept();
		}
		if (ClientList.NodeUseCount > 0) 
		{
			node = ClientList.pHeadNode;
			while (node)
			{
				if (FD_ISSET(node->Socket, &readfds)) // If there has new pocket
				{
					if (Receive(node) == false)
					{
						del_node = node;
						node = node->pNext;
						CloseSocket(del_node->Socket);
						ClientList.Unlink(del_node);
						ClientList.FreeNode(del_node);
						continue;
					}
				}
				node = node->pNext;
			}
		}
	}
}

void TCPServer::Accept()
{
	SOCKET socket = accept(ListenSocket, NULL, NULL);
	if (socket > 0)
	{
		if (ClientList.NodeLinkCount >= FD_SETSIZE)
		{
			closesocket(socket);
			Logger::Log("warning : ClientList.NodeLinkCount(%d) is full. FD_SETSIZE= %d", ClientList.NodeLinkCount, FD_SETSIZE);
			return;
		}
		TDNode< SocketContext >* node = ClientList.GetNode();
		node->Socket = socket;
		ClientList.Link(node);
		Logger::Log("accept ok, create a new connection, socket= %d", node->Socket);
	}
	else
	{
		Logger::Log("accept failed, error: %d", WSAGetLastError());
	}
}

bool TCPServer::Receive( SocketContext* node )
{
	char recv_buff[ DEFAULT_BUFF_LENGTH ] = {0};
	int  recv_buff_len = DEFAULT_BUFF_LENGTH;

	int recv_len = recv( node->Socket, recv_buff, recv_buff_len, 0 );
	if( recv_len > 0 )
	{
		Logger::Log( "bytes recv_len= %d, recv_buff= %s", recv_len, recv_buff );

		int send_result = send( node->Socket, recv_buff, recv_len, 0 );
		if( send_result == SOCKET_ERROR )
		{
			Logger::Log( "send failed, socket= %d, error: %d", node->Socket, WSAGetLastError() );
			return false;
		}
		Logger::Log( "bytes sent: %d", send_result );
		return true;
	}
	else if( recv_len == 0 )
	{
		Logger::Log( "connection closed, socket= %d", node->Socket );
	}
	else
	{
		int error = WSAGetLastError();
		Logger::Log( "recv failed, socket= %d, error: %d", node->Socket, error );
	}
	return false;
}

void TCPServer::CloseSocket(SOCKET& socket)
{
	if (socket != INVALID_SOCKET)
	{
		closesocket(socket);
		socket = 0;
	}
}

void TCPServer::ShutdownAndCloseSocket()
{
	TDNode< SocketContext >* node = ClientList.pHeadNode, * del_node = NULL;
	while (node)
	{
		if (node->Socket != INVALID_SOCKET)
		{
			int result = shutdown(node->Socket, SD_SEND);
			if (result == SOCKET_ERROR)
			{
				Logger::Log("shutdown failed, socket= %d, error: %d", node->Socket, WSAGetLastError());
			}
			closesocket(node->Socket);
		}
		node = node->pNext;
	}

	node = ClientList.pHeadNode;
	while (node)
	{
		del_node = node;
		node = node->pNext;
		ClientList.FreeNode(del_node);
	}
}

void TCPServer::Release()
{
	RunFlag = false;
	shutdown(ListenSocket, SD_SEND);
	CloseSocket(ListenSocket);

	ShutdownAndCloseSocket();

	WSACleanup();

	MainMemoryPool::sMemoryPoolRelease();
}

void TCPServer::Run()
{
	if (CreateSucceedFlag)
	{
		Select();
	}
}