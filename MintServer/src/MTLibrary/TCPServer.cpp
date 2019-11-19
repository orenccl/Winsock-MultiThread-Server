#include "mtpch.h"
#include "MTLibrary/TCPServer.h"
#include "MTLibrary/Tool.h"

bool TCPServer::Create(WORD port)
{
	Logger::Create();

	const UINT memory_pool_bytes_max = 100 * MB;
	MainMemoryPool::sMemoryPoolCreate(memory_pool_bytes_max);

	if (CreateWinsock(port) == false)
		return false;

	// Distribute memory to every List.
	AcceptList.Create(ACCEPT_LIST_MAX);
	SocketRecvList.Create(CONNECTION_MAX);
	SocketSendList.Create(CONNECTION_MAX);
	PacketRecvList.Create(RECEIVE_LIST_MAX);
	PacketSendList.Create(SEND_LIST_MAX);
	ClientList.Create(CONNECTION_MAX);

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

void TCPServer::SetTCPNoDelay(SOCKET socket)
{
	// Turn off Nagle's Alogorithm ( Speed up transmition )
	const char optval_enable = 1;
	int ret = ::setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, &optval_enable, sizeof(optval_enable));
	if (ret == -1)
	{
		Logger::Log("setsockopt( TCP_NODELAY ) failed, error: %d", ::WSAGetLastError());
	}
}

void TCPServer::MainRun() // main thread - ¥D°õ¦æºü
{
	if (CreateSucceedFlag)
	{
		Select_InMainThread(); // Incharge select - accept and recv event.
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

	SOCKET_NODE* socket_node = NULL, * del_socket_node = NULL;

	Logger::Log("TCP server - select mode - multi-threading");

	while (RunFlag)
	{
		// Clear fds_read array
		FD_ZERO(&fds_read);

		// Put listensocket into fds_read array
		FD_SET(ListenSocket, &fds_read);

		// Put client socket into fds_read array
		socket_node = SocketRecvList.pHeadNode;
		while (socket_node)
		{
			FD_SET(socket_node->Socket, &fds_read);
			socket_node = socket_node->pNext;
		}

		// select will wait until recv event occur
		// timeout time is the period program return from select function
		if (AcceptThread.GetSelectTimeModeFlag())
		{
			AcceptListCreateClientNode();
			result = ::select(0, &fds_read, NULL, NULL, &timeout); // When happen accept event, select go into timer mode, until CreateClientNode finish.
		}
		else
			result = ::select(0, &fds_read, NULL, NULL, NULL);     // If nothing to do, blocking.

		if (result == SOCKET_ERROR)
		{
			Logger::Log("select failed, error: %d", ::WSAGetLastError());
		}

		if (FD_ISSET(ListenSocket, &fds_read))    // Check if new connect event occur
		{
			::printf("select_wait... ");
			AcceptThread.SetSelectTimeModeFlag(true);     // Activate select timer mode, deal with accept event
			AcceptThread.Awake();                 // Wake up sAcceptThreadProc
		}

		socket_node = SocketRecvList.pHeadNode;
		while (socket_node)
		{
			if (FD_ISSET(socket_node->Socket, &fds_read)) // Check if receive event occur.
			{
				if (ReceiveEx(socket_node->Socket) == false)
				{
					del_socket_node = socket_node;
					socket_node = socket_node->pNext;
					RecvFreeClientNode(del_socket_node);
					continue;
				}
			}
			socket_node = socket_node->pNext;
		}
	}
	Logger::Log("Select_InMainThread END.");
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
					if (AcceptList.LockGetHeadNode() == NULL)	   // All AcceptList are finished.
					{
						AcceptThread.SetSelectTimeModeFlag(false); // Set select into block mode
						break; // End accept polling
					}
					::Sleep(1);
					continue;
				}
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
			SOCKET_NODE* node = AcceptList.LockGetNode();
			node->Socket = socket;
			::inet_ntop(AF_INET, &client_addr.sin_addr, node->IP, LENGTH_16); // Turn client address into IP string.
			AcceptList.LockLink(node);

			Logger::Log("accept ok, create a new connection, socket= %d, IP= %s", node->Socket, node->IP);
		}
	}

	AcceptThread.Release();
	Logger::Log("Accept_InThread END.");
}

void TCPServer::AcceptListCreateClientNode()
{
	SOCKET_NODE* accept_node = NULL;

	while (AcceptList.NodeLinkCount)
	{
		accept_node = AcceptList.LockGetHeadNode();

		SOCKET_NODE* socket_node = SocketRecvList.GetNode();
		socket_node->Socket = accept_node->Socket;
		Tool::sMemcpy(socket_node->IP, LENGTH_16, accept_node->IP, LENGTH_16);
		SocketRecvList.Link(socket_node);

		NoticeGameplayThreadCreateClientNode(accept_node);

		AcceptList.LockUnlinkFreeNode(accept_node);
	}
}

void TCPServer::RecvFreeClientNode(SOCKET_NODE*& del_socket_node)
{
	SOCKET socket = del_socket_node->Socket;

	SocketRecvList.Unlink(del_socket_node);
	SocketRecvList.FreeNode(del_socket_node);

	SocketSendList.Locker.Lock();   // lock
	SOCKET_SEND_NODE* socket_send_node = SocketSendList.pHeadNode, * del_node = NULL;
	while (socket_send_node)
	{
		if (socket_send_node->Socket == socket)
		{
			del_node = socket_send_node;
			break;
		}
		socket_send_node = socket_send_node->pNext;
	}

	if (del_node)
	{
		SocketSendList.Unlink(del_node);
		SocketSendList.FreeNode(del_node);
	}
	else
	{
		D_WARNING();
	}
	SocketSendList.Locker.Unlock(); // unlock

	NoticeGameplayThreadFreeClientNode(socket);
}

void TCPServer::CreateClientNode(RECV_PACKET_NODE* packet_node)
{
	SOCKET_CLIENT_NODE* client_node = ClientList.GetNode();

	::memset(client_node, 0, sizeof(SOCKET_CLIENT_NODE));
	client_node->Socket = packet_node->Socket;
	Tool::sMemcpy(client_node->IP, LENGTH_16, packet_node->Packet, LENGTH_16);
	ClientList.Link(client_node);

	vUserCreateClientNode(client_node);

	SetTCPNoDelay(client_node->Socket);

	SOCKET_SEND_NODE* socket_send_node	= SocketSendList.LockGetNode();
	socket_send_node->Socket			= client_node->Socket;
	socket_send_node->pS2C_PacketOrderNumber = &(client_node->S2C_PacketOrderNumber);
	SocketSendList.LockLink(socket_send_node);
}

void TCPServer::FreeClientNode(RECV_PACKET_NODE* packet_node)
{
	SOCKET_CLIENT_NODE* client_node = ClientList.pHeadNode, * del_client_node = NULL;
	SOCKET_SEND_NODE* socket_send_node = SocketSendList.LockGetHeadNode(), * del_socket_send_node = NULL;;

	while (client_node)
	{
		if (client_node->Socket == packet_node->Socket)
		{
			del_client_node = client_node;
			break;
		}
		client_node = client_node->pNext;
	}

	while (socket_send_node)
	{
		if (socket_send_node->Socket == packet_node->Socket)
		{
			del_socket_send_node = socket_send_node;
			break;
		}
		socket_send_node = socket_send_node->pNext;
	}


	D_CHECK(del_client_node);

	vUserFreeClientNode(del_client_node);

	if (del_client_node->AliveTimeoutFlag == false)
	{
		CloseSocket(del_client_node->Socket); // Close client socket
	}

	SocketSendList.LockUnlinkFreeNode(del_socket_send_node);
	ClientList.Unlink(del_client_node);
	ClientList.FreeNode(del_client_node);
}

void TCPServer::NoticeGameplayThreadCreateClientNode(SOCKET_NODE* accept_node)
{
	RECV_PACKET_NODE* node = PacketRecvList.LockGetNode();
	node->ActionType = ACTION__CREATE_CLIENT;
	node->Socket = accept_node->Socket;
	Tool::sMemcpy(node->Packet, PACKET_BUFFER_MAX, accept_node->IP, LENGTH_16);
	PacketRecvList.LockLink(node);

	GameplayThread.Awake(); // Awake GameplayThreadProc
}

void TCPServer::NoticeGameplayThreadFreeClientNode(SOCKET socket)
{
	RECV_PACKET_NODE* node = PacketRecvList.LockGetNode();
	node->ActionType = ACTION__RELEASE_CLIENT;
	node->Socket = socket;
	PacketRecvList.LockLink(node);

	GameplayThread.Awake(); // Awake GameplayThreadProc
}

bool TCPServer::ReceiveEx(SOCKET socket)
{
	int  recv_packet_max = PACKET_LENGTH_SIZE;
	int  recv_length = 0;
	int  buff_packet_index = 0;

	int  recv_retry_count = 0;
	const int recv_retry_max = 5000;

	RECV_PACKET_NODE* packet_node = PacketRecvList.LockGetNode();

	while (recv_packet_max > 0)
	{
		recv_length = ::recv(socket, &packet_node->Packet[buff_packet_index], recv_packet_max, 0);
		if (recv_length > 0)
		{
			recv_packet_max -= recv_length;
			buff_packet_index += recv_length;
		}
		else if (recv_length == 0)
		{
			Logger::Log("RecvEx #1 recv notice, client connection closed, socket= %d", socket);
			PacketRecvList.LockFreeNode(packet_node);
			return false;
		}
		else
		{
			int error = ::WSAGetLastError();
			if (error == WSAEWOULDBLOCK) // 10035 - recv buffer has no more package
			{
				if (++recv_retry_count < recv_retry_max)
				{
					::Sleep(1);
					continue;
				}
				Logger::Log("RecvEx #1 recv failed, WSAEWOULDBLOCK, recv_retry_count full, socket= %d, error: %d", socket, error);
				PacketRecvList.LockFreeNode(packet_node);
				return false; // This could receive package anymore.
			}
			Logger::Log("RecvEx #1 recv failed, socket= %d, error: %d", socket, error);
			PacketRecvList.LockFreeNode(packet_node);
			return false;
		}
	}

	HeadPacketInfo* head_packet = (HeadPacketInfo*)packet_node->Packet;
	recv_packet_max = head_packet->PacketLength - PACKET_LENGTH_SIZE;

	while (recv_packet_max > 0)
	{
		// Receive until get a complete package
		recv_length = ::recv(socket, &packet_node->Packet[buff_packet_index], recv_packet_max, 0);
		if (recv_length > 0)
		{
			recv_packet_max -= recv_length;
			buff_packet_index += recv_length;
		}
		else if (recv_length == 0)
		{
			Logger::Log("RecvEx #2 recv notice, client connection closed, socket= %d", socket);
			PacketRecvList.LockFreeNode(packet_node);
			return false;
		}
		else
		{
			int error = ::WSAGetLastError();
			if (error == WSAEWOULDBLOCK) // 10035 - recv buffer has no more package
			{
				if (++recv_retry_count < recv_retry_max)
				{
					::Sleep(1);
					continue;
				}
				Logger::Log("RecvEx #2 recv failed, WSAEWOULDBLOCK, recv_retry_count full, socket= %d, error: %d", socket, error);
				PacketRecvList.LockFreeNode(packet_node);
				return false;
			}
			Logger::Log("RecvEx #2 recv failed, socket= %d, error: %d", socket, error);
			PacketRecvList.LockFreeNode(packet_node);
			return false;
		}
	}

	if (recv_packet_max == 0)
	{
		packet_node->ActionType = ACTION__PACKET;
		packet_node->Socket = socket;
		packet_node->PacketLength = head_packet->PacketLength;
		PacketRecvList.LockLink(packet_node);
		GameplayThread.Awake(); // Awake sGameplayThreadProc
	}
	else
	{
		static int s_count = 0;
		Logger::Log("RecvEx #3 socket= %d, ERROR packet_length= %d, s_count= %d", socket, head_packet->PacketLength, ++s_count);
		PacketRecvList.LockFreeNode(packet_node);
	}

	return true;
}

void TCPServer::Gameplay_InThread()
{
	SOCKET_CLIENT_NODE* client_node = NULL;
	RECV_PACKET_NODE* packet_node = NULL;
	PacketBuffer* packet = NULL;

	while (RunFlag)
	{
		GameplayThread.Wait(); // Block wait
		while (packet_node = PacketRecvList.LockGetHeadNode())
		{
			client_node = GetSocketClientNode(packet_node->Socket); // TODO : index_table for quick search.
			packet = (PacketBuffer*)packet_node->Packet;

			if (packet && ::strcmp(packet->Buffer, "server_exit") == 0) // test : client shut server down.
			{
				PacketRecvList.LockUnlinkFreeNode(packet_node);
				ServerExit();
				break;
			}

			switch (packet_node->ActionType)
			{
				case ACTION__CREATE_CLIENT:
				{
					CreateClientNode(packet_node);  // Initialize player data
					break;
				}
				case ACTION__RELEASE_CLIENT:
				{
					FreeClientNode(packet_node);
					break;
				}
				case ACTION__PACKET:
				{
					D_CHECK_BREAK(client_node);
					D_CHECK_BREAK(packet);

					// Check if package order number are the same.
					if (packet->PacketOrderNumber == ++client_node->C2S_PacketOrderNumber)
					{
						if (packet->PacketType == PACKET__C2S_ALIVE)
							Logger::Log("Gameplay   socket= %d, packet_length= %d, [%d], alive packet.", packet_node->Socket, packet_node->PacketLength, packet->PacketOrderNumber);
						else
							Logger::Log("Gameplay   socket= %d, packet_length= %d, [%d], buffer= %s", packet_node->Socket, packet_node->PacketLength, packet->PacketOrderNumber, packet->Buffer);
					}
					else
					{
						// Package order number are not the same.
						Logger::Log("Gameplay   ERROR PacketOrderNumber : socket= %d, packet_length= %d, client[%d], server[%d], buffer= %s", packet_node->Socket, packet_node->PacketLength, packet->PacketOrderNumber, client_node->C2S_PacketOrderNumber, packet->Buffer);
						CloseSocket(client_node->Socket); // Cut off connection.
						break;
					}

					// Clear hearthbeat
					client_node->AliveTimeout = {};

					switch (packet->PacketType)
					{
						case PACKET__C2S_ALIVE: // hearthbeat package
						{
							break;
						}
						default:
						{
							// GameServer can deal with messenge adter received
							vOnGameplayReceivePacket(client_node, packet_node, packet);
							break;
						}
					}
					CheckClientAlive();
					break;
				}
			}

			PacketRecvList.LockUnlinkFreeNode(packet_node);
		}
	}

	GameplayThread.Release();
	Logger::Log("Gameplay_InThread END.");
}

SOCKET_CLIENT_NODE* TCPServer::GetSocketClientNode(SOCKET socket)
{
	SOCKET_CLIENT_NODE* client_node = ClientList.pHeadNode;
	while (client_node)
	{
		if (client_node->Socket == socket)
		{
			return client_node;
		}
		client_node = client_node->pNext;
	}
	return NULL;
}

void TCPServer::CheckClientAlive()
{
	const UINT delay_time = 60 * MILLI_SECOND; // 60s
	SOCKET_CLIENT_NODE* client_node = ClientList.pHeadNode;
	while (client_node)
	{
		if (Tool::sDelayTime(client_node->AliveTimeout.StartTime, client_node->AliveTimeout.EndTime, delay_time))
		{
			Logger::Log("client timeout, close socket= %d", client_node->Socket);
			client_node->AliveTimeoutFlag = true;
			// Close socket，trigger recv event，let FreeClientNode() can release player data.
			::closesocket(client_node->Socket);
		}
		client_node = client_node->pNext;
	}
}

void TCPServer::Send_InThread()
{
	SEND_PACKET_NODE*	packet_node = NULL;
	SOCKET_SEND_NODE*	socket_send_node = NULL;
	SocketSendInfo*		socket_client = NULL;
	HeadPacketInfo*		head_packet = NULL;

	while (RunFlag)
	{
		SendThread.Wait();
		while (packet_node = PacketSendList.LockGetHeadNode())
		{
			if (SocketSendCache.SocketTableUseCount != SocketSendList.NodeLinkCount)
			{
				// Optimize：Put socket list into socket cache table lower down lock competition
				SocketSendList.Locker.Lock();   // lock
				SocketSendCache.SocketTableUseCount = 0;
				socket_send_node = SocketSendList.pHeadNode;
				while (socket_send_node)
				{
					socket_client = &SocketSendCache.SocketTable[SocketSendCache.SocketTableUseCount];
					socket_client->Socket					= socket_send_node->Socket;
					socket_client->pS2C_PacketOrderNumber	= socket_send_node->pS2C_PacketOrderNumber;
					++SocketSendCache.SocketTableUseCount;
					socket_send_node = socket_send_node->pNext;
				}
				SocketSendList.Locker.Unlock(); // unlock
			}

			head_packet = (HeadPacketInfo*)packet_node->Packet;

			switch (packet_node->SendType)
			{
				case SEND__ALL:
				{
					const int max = SocketSendCache.SocketTableUseCount;
					for (int i = 0; i < max; ++i)
					{
						socket_client = &SocketSendCache.SocketTable[i];
			
						head_packet->PacketOrderNumber = ++(*socket_client->pS2C_PacketOrderNumber);
						SendEx(socket_client->Socket, packet_node->Packet, packet_node->PacketLength);
					}
					break;
				}
				case SEND__SELF:
				{
					break;
				}
				case SEND__NON_SELF:
				{
					const int max = SocketSendCache.SocketTableUseCount;
					for (int i = 0; i < max; ++i)
					{
						if (packet_node->Socket == (SocketSendCache.SocketTable[i]).Socket)
							continue;
						socket_client = &SocketSendCache.SocketTable[i];

						head_packet->PacketOrderNumber = ++(*socket_client->pS2C_PacketOrderNumber);
						SendEx(socket_client->Socket, packet_node->Packet, packet_node->PacketLength);
					}
					break;
				}
				case SEND__GROUP_ALL:
				{
					break;
				}
				case SEND__GROUP_SELF:
				{
					break;
				}
				case SEND__GROUP_NON_SELF:
				{
					break;
				}
			}

			PacketSendList.LockUnlinkFreeNode(packet_node);
		}
	}

	SendThread.Release();
	Logger::Log("Send_InThread END.");
}

void TCPServer::AddSendList(_SEND_TYPE_ send_type, SOCKET_CLIENT_NODE* client_node, HeadPacketInfo* packet)
{
	SEND_PACKET_NODE* send_packet_node	= PacketSendList.LockGetNode();
	send_packet_node->SendType			= send_type;
	send_packet_node->Socket			= client_node->Socket;
	send_packet_node->PacketLength		= packet->PacketLength;
	Tool::sMemcpy(send_packet_node->Packet, PACKET_BUFFER_MAX, packet, packet->PacketLength);
	PacketSendList.LockLink(send_packet_node);
	SendThread.Awake(); // Awake SendThreadProc
}

bool TCPServer::SendEx(SOCKET socket, char* packet, WORD packet_length)
{
	int  send_packet_max = packet_length;
	int  send_length = 0;
	int  buff_packet_index = 0;

	int  send_retry_count = 0;
	const int send_retry_max = 5000;

	while (send_packet_max > 0)
	{
		send_length = ::send(socket, &packet[buff_packet_index], send_packet_max, 0);
		if (send_length > 0)
		{
			send_packet_max -= send_length;
			buff_packet_index += send_length;
		}
		else
		{
			int error = ::WSAGetLastError();
			if (error == WSAEWOULDBLOCK) // 10035
			{
				if (++send_retry_count < send_retry_max)
				{
					::Sleep(1);
					continue;
				}
				Logger::Log("send failed, WSAEWOULDBLOCK, send_retry_count full, socket= %d, error: %d", socket, error);
				return false; // 此socket可能已經發生錯誤.
			}
			Logger::Log("send failed, socket= %d, error: %d", socket, error);
			return false;
		}
	}
	Logger::Log("send okay, send_length: %d, socket= %d", send_length, socket);
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

void TCPServer::ServerExit()
{
	RunFlag = false;

	CloseSocket(ListenSocket); // Shutdown ListenSocket，select will be wake up and do close task.

	// 每個thread都喚醒，進行關閉流程。
	AcceptThread.Awake();
	GameplayThread.Awake();
	SendThread.Awake();
}

void TCPServer::ShutdownAndCloseSocket()
{
	SOCKET_CLIENT_NODE* client_node = ClientList.pHeadNode, * del_client_node = NULL;
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
	AcceptThread.ReleaseWait();
	GameplayThread.ReleaseWait();
	SendThread.ReleaseWait();

	ShutdownAndCloseSocket();

	::WSACleanup();

	MainMemoryPool::sMemoryPoolRelease();
	MemoryCounter::sShow_MemoryUseCount();
}