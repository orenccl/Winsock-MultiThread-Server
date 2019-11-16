#include "mtpch.h"
#include "MTLibrary/TCPServer.h"
#include "MTLibrary/Tool.h"

#pragma comment( lib, "WS2_32.lib" ) // winsock2.h

TCPServer::TCPServer()
{
	::memset(this, 0, sizeof(TCPServer)); // Initial entire TCPServer to zero.
}

bool TCPServer::Create(WORD port)
{
	const UINT memory_pool_bytes_max = 100 * MB;
	MainMemoryPool::sMemoryPoolCreate(memory_pool_bytes_max);

	if (CreateWinsock(port) == false)
		return false;

	// Distribute memory to every List.
	AcceptList.Create(ACCEPT_LIST_MAX);
	ClientList.Create(CONNECTION_MAX);
	SocketList.Create(CONNECTION_MAX);
	ReceiveList.Create(RECEIVE_LIST_MAX);
	SendList.Create(SEND_LIST_MAX);

	CreateSucceedFlag = true;
	RunFlag = true;

	Logger::Log("TCPServer::Create Succeed.");

	// Initialize thread
	AcceptThread.Create(sAcceptThreadProc, this);
	GameplayThread.Create(sGameplayThreadProc, this);
	SendThread.Create(sSendThreadProc, this);

	return true;
}

bool TCPServer::CreateWinsock(WORD port)
{
	WSADATA wsa_data = { 0 };
	int result = 0;

	// Winsock version 2.2
	result = ::WSAStartup(0x202, &wsa_data);
	if (result != 0)
	{
		Logger::Log("WSAStartup failed, error: %d", result);
		return false;
	}

	// Create a local listen socket, to accept client's connection
	// AF_INET : IPv4 Internet Protocol
	// SOCK_STREAM : Data streaming, corresponding to TCP transmit protocal
	// IPPROTO_TCP : Use TCP transmit Protocal
	ListenSocket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ListenSocket == INVALID_SOCKET)
	{
		Logger::Log("socket failed, error: %ld", ::WSAGetLastError());
		return false;
	}

	// Setting Address Info
	sockaddr_in addr = { 0 };
	// Use IPv4 Internet Protocol
	addr.sin_family = AF_INET;
	// Set port, prepare a port to listen
	//( use htons() function to transform is because Network Byte Order may be different to Host Byte Order
	addr.sin_port = htons(port);
	// Set address to 0.0.0.0, means it can receive all package from binding port
	// regardless Network Cark address
	addr.sin_addr.s_addr = INADDR_ANY;

	// Bind to ListenSocket, if binding fail, reason could be port occupied.
	result = ::bind(ListenSocket, (SOCKADDR*)&addr, sizeof(addr));
	if (result == SOCKET_ERROR)
	{
		Logger::Log("bind failed, error: %d, port= %d", ::WSAGetLastError(), port);
		return false;
	}

	// Enable listen, could receive connection from client
	// Second parm is use to set maximum listen list, SOMAXCONN means system maximum
	result = ::listen(ListenSocket, SOMAXCONN);
	if (result == SOCKET_ERROR)
	{
		Logger::Log("listen failed, error: %d", ::WSAGetLastError());
		return false;
	}

	// Set ListenSocket to non-blocking mode
	// FIONBIO : set/clear non-blocking i/o
	unsigned long non_blocking_mode = 1; // ­1 = non-blocking
	result = ::ioctlsocket(ListenSocket, FIONBIO, &non_blocking_mode);
	if (result == SOCKET_ERROR)
	{
		printf("ioctlsocket failed, error: %d", ::WSAGetLastError());
		return false;
	}

	return true;
}

void TCPServer::MainRun()
{
	if (CreateSucceedFlag)
	{
		Select_InMainThread(); // Deal with select - accept, recv.
	}
}

UINT WINAPI TCPServer::sAcceptThreadProc(LPVOID param)
{
	Logger::Log("TCPServer::sAcceptThreadProc running...");
	TCPServer* server = (TCPServer*)param;
	server->Accept_InThread();
	return 0;
}

UINT WINAPI TCPServer::sGameplayThreadProc(LPVOID param)
{
	Logger::Log("TCPServer::sGameplayThreadProc running...");
	TCPServer* server = (TCPServer*)param;
	server->Gameplay_InThread();
	return 0;
}

UINT WINAPI TCPServer::sSendThreadProc(LPVOID param)
{
	Logger::Log("TCPServer::sSendThreadProc running...");
	TCPServer* server = (TCPServer*)param;
	server->Send_InThread();
	return 0;
}

void TCPServer::Select_InMainThread()
{
	// Initialize socket array
	fd_set fds_read = { 0 };

	const long milliseconds =  100 * MILLI_SECOND;

	/*!
	* @param0 : tv_sec = second.
	* @param1 : tv_usec = units. 0 = ms, 1 = s.
	*/
	timeval timeout = { 0, milliseconds };
	int result = 0;

	TSNode< SocketInfo >* accept_node = NULL;
	TDNode< SocketContext >* client_node = NULL, * del_client_node = NULL;

	Logger::Log("TCP server - select mode - multi-threading");

	while (RunFlag)
	{
		while (AcceptList.NodeLinkCount)
		{
			accept_node = AcceptList.LockGetHeadNode();

			CreateClientNode(accept_node); // Initial client data

			AcceptList.LockUnlinkFreeNode(accept_node);
		}

		// Clear fds_read array
		FD_ZERO(&fds_read);

		// Put listensocket into fds_read array
		FD_SET(ListenSocket, &fds_read);

		// Put client socket into fds_read array
		client_node = ClientList.pHeadNode;
		while (client_node)
		{
			FD_SET(client_node->Socket, &fds_read);
			client_node = client_node->pNext;
		}

		// select will wait until recv event occur
		// timeout time is the period program return from select function
		if (AcceptThread.GetSwitchFlag())
			result = ::select(0, &fds_read, NULL, NULL, &timeout); // When happen accept event, select go into timer mode, until CreateClientNode finish.
		else
			result = ::select(0, &fds_read, NULL, NULL, NULL);     // If nothing to do, blocking.

		if (result == SOCKET_ERROR)
		{
			RunFlag = false;
			AcceptThread.Start();
			Logger::Log("select failed, error: %d", ::WSAGetLastError());
			break;
		}

		if (FD_ISSET(ListenSocket, &fds_read))    // Check if new connect event occur
		{
			::printf("select_wait... ");
			AcceptThread.SetSwitchFlag(true);     // Activate select timer mode, deal with accept event
			AcceptThread.Start();                 // Wake up sAcceptThreadProc
		}

		client_node = ClientList.pHeadNode;
		while (client_node)
		{
			if (FD_ISSET(client_node->Socket, &fds_read)) // Check if receive event occur.
			{
				if (Receive(client_node) == false)
				{
					del_client_node = client_node;
					client_node = client_node->pNext;
					FreeClientNode(del_client_node);
					continue;
				}
			}
			client_node = client_node->pNext;
		}
	}
}

void TCPServer::Accept_InThread()
{
	// accept - non-blocking

	int error = 0;
	SOCKET socket = 0;
	sockaddr_in client_addr;
	int client_addr_size = sizeof(client_addr);

	while (RunFlag)
	{
		AcceptThread.Wait();
		while (RunFlag)
		{
			socket = ::accept(ListenSocket, (SOCKADDR*)&client_addr, &client_addr_size);
			if (socket == INVALID_SOCKET)
			{
				error = ::WSAGetLastError();
				if (error == WSAEWOULDBLOCK) // 10035 = No new connect request.
				{
					AcceptThread.SetSwitchFlag(false); // Set select into block mode
					break;
				}
				// Unexpected error, turn down server.
				RunFlag = false;
				CloseSocket(ListenSocket); // Trun off ListenSocket, and wake up select.
				Logger::Log("accept failed, error: %d", error);
				break;
			}

			if (ClientList.NodeLinkCount >= ClientList.ObjectCountMax)
			{
				CloseSocket(socket);
				Logger::Log("warning : ClientList.NodeLinkCount(%d) is full. ClientList.ObjectCountMax= %d", ClientList.NodeLinkCount, ClientList.ObjectCountMax);
				continue;
			}

			// Put new connection in to AcceptList
			TSNode< SocketInfo >* node = AcceptList.LockGetNode();
			node->Socket = socket;
			::inet_ntop(AF_INET, &client_addr.sin_addr, node->IP, LENGTH_16); // Turn client address into IP string.
			AcceptList.LockLink(node);

			Logger::Log("accept ok, create a new connection, socket= %d, IP= %s", node->Socket, node->IP);
		}
	}

	AcceptThread.Release();
}

void TCPServer::CreateClientNode(TSNode< SocketInfo >* accept_node)
{
	TDNode< SocketContext >* client_node = ClientList.GetNode();
	client_node->Socket = accept_node->Socket;
	Tool::sMemcpy(client_node->IP, LENGTH_16, accept_node->IP, LENGTH_16);
	ClientList.Link(client_node);

	SocketList.Locker.Lock();   // lock
	TDNode< SocketInfo >* socket_node = SocketList.GetNode();
	socket_node->Socket = accept_node->Socket;
	Tool::sMemcpy(socket_node->IP, LENGTH_16, accept_node->IP, LENGTH_16);
	SocketList.Link(socket_node);
	SocketList.Locker.Unlock(); // unlock
}

void TCPServer::FreeClientNode(TDNode< SocketContext >*& client_node)
{
	SocketList.Locker.Lock();   // lock
	TDNode< SocketInfo >* socket_node = SocketList.pHeadNode, * del_socket_node = NULL;
	while (socket_node)
	{
		if (socket_node->Socket == client_node->Socket)
		{
			del_socket_node = socket_node;
			break;
		}
		socket_node = socket_node->pNext;
	}
	if (del_socket_node)
	{
		SocketList.Unlink(del_socket_node);
		SocketList.FreeNode(del_socket_node);
	}
	SocketList.Locker.Unlock(); // unlock

	// Release client socket
	CloseSocket(client_node->Socket);

	ClientList.Unlink(client_node);
	ClientList.FreeNode(client_node);
}

bool TCPServer::Receive(SocketContext* node)
{
	TSNode< RecvPacketInfo >* packet_node = ReceiveList.LockGetNode();

	int recv_length = ::recv(node->Socket, packet_node->Packet, PACKET_BUFFER_MAX, 0);
	if (recv_length > 0)
	{
		PacketInfo* packet = (PacketInfo*)packet_node->Packet;
		Logger::Log("socket= %d, bytes recv_length= %d, [%d] %s¡G%s", node->Socket, recv_length, packet->Number, packet->Name, packet->Message);
		packet_node->PacketLength = recv_length;
		packet_node->Socket = node->Socket;
		ReceiveList.LockLink(packet_node);
		GameplayThread.Start(); // Wake up GameplayThreadProc
		return true;
	}
	ReceiveList.LockFreeNode(packet_node);

	if (recv_length == 0)
	{
		Logger::Log("client connection closed, socket= %d", node->Socket);
	}
	else
	{
		int error = ::WSAGetLastError();
		Logger::Log("recv failed, socket= %d, error: %d", node->Socket, error);
	}
	return false;
}

void TCPServer::Gameplay_InThread()
{
	PacketInfo* packet = NULL;
	TSNode< RecvPacketInfo >* packet_node = NULL;

	while (RunFlag)
	{
		GameplayThread.Wait();
		while (packet_node = ReceiveList.LockGetHeadNode())
		{
			packet = (PacketInfo*)packet_node->Packet;
			if (strcmp(packet->Message, "server_exit") == 0) // temp
			{
				RunFlag = false;
				CloseSocket(ListenSocket); // Turn off listen socket, Wake select.
				AcceptThread.Start();
				break;
			}

			// Deal with logical tasks after receive packet.
			vOnGameplayReceivePacket(packet_node);

			ReceiveList.LockUnlinkFreeNode(packet_node);
		}
	}

	GameplayThread.Release();
}

void TCPServer::Send_InThread()
{
	TDNode< SocketInfo >* socket_node = NULL;
	TSNode< SendPacketInfo >* packet_node = NULL;

	while (RunFlag)
	{
		SendThread.Wait();
		while (packet_node = SendList.LockGetHeadNode())
		{
			switch (packet_node->SendType)
			{
			case SEND__ALL:
			{
				SocketList.Locker.Lock();   // lock
				socket_node = SocketList.pHeadNode;
				while (socket_node)
				{
					Send(socket_node->Socket, packet_node->Packet, packet_node->PacketLength);
					socket_node = socket_node->pNext;
				}
				SocketList.Locker.Unlock(); // unlock
				break;
			}
			case SEND__SELF:
			{
				break;
			}
			case SEND__NON_SELF:
			{
				break;
			}
			}

			SendList.LockUnlinkFreeNode(packet_node);
		}
	}

	SendThread.Release();
}

void TCPServer::AddSendList(_SEND_TYPE_ send_type, SOCKET socket, char* packet, WORD packet_length)
{
	TSNode< SendPacketInfo >* send_packet_node = SendList.LockGetNode();
	send_packet_node->Socket = socket;
	send_packet_node->SendType = send_type;
	send_packet_node->PacketLength = packet_length;
	Tool::sMemcpy(send_packet_node->Packet, PACKET_BUFFER_MAX, packet, packet_length);
	SendList.LockLink(send_packet_node);
	SendThread.Start(); // Wake up SendThreadProc
}

bool TCPServer::Send(SOCKET socket, const char* packet, WORD packet_length)
{
	int send_length = ::send(socket, packet, packet_length, 0);
	if (send_length > 0)
	{
		Logger::Log("socket= %d, bytes send_length: %d", socket, send_length);

		return true;
	}

	int error = ::WSAGetLastError();
	Logger::Log("send failed, socket= %d, error: %d", socket, error);
	return false;
}

void TCPServer::CloseSocket(SOCKET& socket)
{
	if (socket != INVALID_SOCKET)
	{
		::closesocket(socket);
		socket = INVALID_SOCKET;
	}
}

void TCPServer::ShutdownAndCloseSocket()
{
	TDNode< SocketContext >* client_node = ClientList.pHeadNode, * del_client_node = NULL;
	while (client_node)
	{
		if (client_node->Socket != INVALID_SOCKET)
		{
			// Shutdown connection
			// SD_SEND could not be call after shutdown.
			int result = ::shutdown(client_node->Socket, SD_SEND);
			if (result == SOCKET_ERROR)
			{
				Logger::Log("shutdown failed, socket= %d, error: %d", client_node->Socket, ::WSAGetLastError());
			}
			CloseSocket(client_node->Socket);
		}
		client_node = client_node->pNext;
	}

	client_node = ClientList.pHeadNode;
	while (client_node)
	{
		del_client_node = client_node;
		client_node = client_node->pNext;
		ClientList.FreeNode(del_client_node);
	}
}

void TCPServer::Release()
{
	RunFlag = false;
	CloseSocket(ListenSocket);

	AcceptThread.ReleaseWait();
	GameplayThread.ReleaseWait();
	SendThread.Release();
	SendThread.ReleaseWait();

	ShutdownAndCloseSocket();

	::WSACleanup(); // Release winsock

	MainMemoryPool::sMemoryPoolRelease();
}