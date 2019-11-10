#include "mtpch.h"
#include "MTLibrary/TCPServer.h"

#pragma comment( lib, "WS2_32.lib" ) // winsock2.h

#define DEFAULT_BUFF_LENGTH  512

TCPServer::TCPServer()
{
	ZeroMemory(this, sizeof(TCPServer));
}


bool TCPServer::Create(WORD port)
{
	if (CreateWinsock(port) == false)
		return false;

	ClientList.Create();

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

	ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ListenSocket == INVALID_SOCKET)
	{
		printf("socket failed, error: %ld\n", WSAGetLastError());
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

	unsigned long non_blocking_mode = 1;
	result = ioctlsocket(ListenSocket, FIONBIO, &non_blocking_mode);
	if (result == SOCKET_ERROR)
	{
		Logger::Log("ioctlsocket failed, error: %d", WSAGetLastError());
		return false;
	}

	return true;
}


void TCPServer::Accept()
{
	SOCKET socket = accept(ListenSocket, NULL, NULL);
	if (socket == INVALID_SOCKET)
	{
		int error = WSAGetLastError();
		if (error == WSAEWOULDBLOCK)
			return;
		Logger::Log("accept failed, error: %d", WSAGetLastError());
		return;
	}

	if (ClientList.NodeLinkCount >= CLIENT_MAX)
	{
		CloseSocket(socket);
		Logger::Log("warning : ClientList.NodeLinkCount(%d) is full. CLIENT_MAX= %d", ClientList.NodeLinkCount, CLIENT_MAX);
		return;
	}
	TDNode< SocketContext >* node = ClientList.GetNode();
	node->Socket = socket;
	ClientList.Link(node);
	Logger::Log("accept ok, create a new connection, socket= %d", node->Socket);
}

void TCPServer::Receive()
{
	if (ClientList.NodeUseCount > 0)
	{
		TDNode< SocketContext >* node = ClientList.pHeadNode;
		while (node)
		{
			if (ReceiveOne(node) == false)
			{
				TDNode< SocketContext >* del_node = node;
				node = node->pNext;
				CloseSocket(del_node->Socket);
				ClientList.Unlink(del_node);
				ClientList.ReleaseNode(del_node);
				continue;
			}
			node = node->pNext;
		}
	}
}

bool TCPServer::ReceiveOne(SocketContext* node)
{
	char recv_buff[DEFAULT_BUFF_LENGTH] = { 0 };
	int  recv_buff_len = DEFAULT_BUFF_LENGTH;

	int recv_len = recv(node->Socket, recv_buff, recv_buff_len, 0);
	if (recv_len < 0)
	{
		int error = WSAGetLastError();
		if (error == WSAEWOULDBLOCK)
			return true;
		Logger::Log("recv failed, socket= %d, error: %d", node->Socket, error);
	}
	else if (recv_len > 0)
	{
		Logger::Log("bytes recv_len= %d, recv_buff= %s", recv_len, recv_buff);

		int send_result = send(node->Socket, recv_buff, recv_len, 0);
		if (send_result == SOCKET_ERROR)
		{
			Logger::Log("send failed, socket= %d, error: %d", node->Socket, WSAGetLastError());
			return false;
		}
		Logger::Log("bytes sent: %d", send_result);
		memset(recv_buff, 0, DEFAULT_BUFF_LENGTH);
		return true;
	}
	else if (recv_len == 0)
	{
		Logger::Log("connection closed, socket= %d", node->Socket);
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

void TCPServer::Release()
{
	TDNode< SocketContext >* node = ClientList.pHeadNode, * del_node = NULL;
	while (node)
	{
		int result = shutdown(node->Socket, SD_SEND);
		if (result == SOCKET_ERROR)
		{
			Logger::Log("shutdown failed, socket= %d, error: %d", node->Socket, WSAGetLastError());
		}
		CloseSocket(node->Socket);
		node = node->pNext;
	}

	ClientList.Release();

	CloseSocket(ListenSocket);

	WSACleanup();
}

void TCPServer::Run()
{
	if (CreateSucceedFlag)
	{
		Logger::Log("TCP server - non-blocking mode - multi-client");
		while (RunFlag)
		{
			Accept();
			Receive();
			printf(".");
			Sleep(100);
		}
	}
}