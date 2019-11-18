#include "mtpch.h"
#include "MTLibrary/Packet.h"
#include "MTLibrary/Logger.h"
#include "MTLibrary/MemoryCounter.h"
#include "MTLibrary/MemoryPool.h"
#include "MTLibrary/Task.h"

#define SERVER_PORT    27015

struct SocketClient
{
	SOCKET    Socket;
	char      Name[NAME_BUFFER_MAX];
	WORD      S2C_PacketOrderNumber;
	WORD      C2S_PacketOrderNumber;
	TaskNode* pAliveNode;
};

enum _TASK_TYPE_
{
	TASK__ALIVE = 1,
};

class TCPClient : public TaskInterface
{
	SocketClient    Client;
	PacketBuffer    RecvPacket;
	int             RecvPacketLength;
	int             RecvPacketBufferIndex;

	TaskComponent* TaskManager;

public:
	TCPClient()
		:TaskManager(NULL), RecvPacketLength(0), RecvPacketBufferIndex(0), Client({ 0 }), RecvPacket({0}) {}

	bool Create()
	{
		if (CreateWinsock() == false)
			return false;

		RecvPacketLength = PACKET_LENGTH_SIZE;

		TaskManager = TaskComponent::sCreate(this);
		Client.pAliveNode = TaskManager->vAddTask(TASK__ALIVE, 1000 * 30, TASK_RUN_COUNT_UNLIMITED);

		return true;
	}
	bool CreateWinsock()
	{
		printf("TCP client create\n");

		WSADATA wsaData = { 0 };
		int result = 0;

		// Version 2.2
		result = ::WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (result != 0)
		{
			printf("WSAStartup failed, error: %d\n", result);
			return false;
		}

		// Create a local connect socket, to connect with server.
		// AF_INET : IPv4 Internet Protocol
		// SOCK_STREAM : Data streaming, corresponding to TCP transmit protocal
		// IPPROTO_TCP : Use TCP transmit Protocal
		Client.Socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (Client.Socket == INVALID_SOCKET)
		{
			printf("socket failed, error: %d\n", ::WSAGetLastError());
			return false;
		}

		const char* ip = "127.0.0.1";             // Connect to local IP
		sockaddr_in addr = { 0 };
		addr.sin_family = AF_INET;                // IPv4 Internet protocal
		addr.sin_port = htons(SERVER_PORT);       // Set port
		::inet_pton(AF_INET, ip, &addr.sin_addr); // Set IP for client to connect to.

		result = ::connect(Client.Socket, (SOCKADDR*)&addr, sizeof(addr)); // connect to server
		if (result == SOCKET_ERROR)
		{
			printf("connect failed, error: %d\n", ::WSAGetLastError());
			return false;
		}

		if (Client.Socket == INVALID_SOCKET)
		{
			printf("unable to connect to server !\n");
			return false;
		}
		else
			printf("connect to server !\n");

		// Set ConnectSocket non-blocking
		// FIONBIO : Set or clear non-blocking mode
		unsigned long non_blocking_mode = 1; // ? is set to on-blocking mode
		result = ::ioctlsocket(Client.Socket, FIONBIO, &non_blocking_mode);
		if (result == SOCKET_ERROR)
		{
			printf("ioctlsocket failed, error: %d", ::WSAGetLastError());
			return false;
		}

		const char optval_enable = 1;
		int ret = ::setsockopt(Client.Socket, IPPROTO_TCP, TCP_NODELAY, &optval_enable, sizeof(optval_enable));
		if (ret == -1)
		{
			printf("setsockopt( TCP_NODELAY ) failed, error: %d", ::WSAGetLastError());
		}

		return true;
	}

	void NameInput()
	{
		int name_length = 0;

		while (1)
		{
			printf("name input : ");
			::fgets(Client.Name, NAME_BUFFER_MAX, stdin); // fgets() blocking wait.
			if (Client.Name[0] == '\n')
				continue;

			name_length = ::strlen(Client.Name); // Get Name length.
			if (name_length > 0)
			{
				Client.Name[name_length - 1] = 0; // Clear \n that cause by fgets().
				name_length -= 1;
				break;
			}
		}

		Send_C2S_Name(Client.Name, name_length);
	}

	void ChatInput()
	{
		int  keyboard_state = 0, ch = 0;
		int  buff_message_length = 0;
		char buff_message[PACKET_BUFFER_MAX] = { 0 };

#define BACKSPCAE_KEY_CODE  8
#define ENTER_KEY_CODE     13

		printf("chat input... \n");

		while (1)
		{
			keyboard_state = ::_kbhit(); // Check keyboard hit event.
			if (keyboard_state > 0)
			{
				ch = ::_getch(); // Get character
				printf("%c", ch);
				if (ch == ENTER_KEY_CODE) // ­When hit enter
				{
					printf("\n");
					if (::strcmp(buff_message, "exit") == 0) // Trun down client itself
						break;

					if (buff_message_length > 0)
					{
						//Send_C2S_Message(buff_message, buff_message_length);
						Send_C2S_Message_2(buff_message, buff_message_length);
						buff_message_length = 0;
						::memset(buff_message, 0, PACKET_BUFFER_MAX);

						Client.pAliveNode->StartTime = 0; // Heartbeattimer
					}
				}
				else if (ch == BACKSPCAE_KEY_CODE)
				{
					if (buff_message_length - 1 >= 0)
						buff_message[--buff_message_length] = 0;
				}
				else // Save messenge
				{
					if (buff_message_length + 1 < PACKET_BUFFER_MAX)
						buff_message[buff_message_length++] = ch;
				}
			}

			if (RecvEx(Client.Socket) == false) // recv(non-blocking)
				break;

			TaskManager->vRunTask();

			::Sleep(1);
		}
	}
	
	void Send_C2S_Alive()
	{
		C2S_Alive packet = {};

		packet.PacketLength = sizeof(HeadPacketInfo);
		packet.PacketType = PACKET__C2S_ALIVE;
		packet.PacketOrderNumber = ++Client.C2S_PacketOrderNumber;

		SendEx(Client.Socket, (char*)&packet, packet.PacketLength);
	}
	
	void Send_C2S_Name(char* buff_name, UINT name_length)
	{
		C2S_Name packet = {};

		packet.PacketLength = sizeof(HeadPacketInfo) + name_length;
		packet.PacketType = PACKET__C2S_NAME;
		packet.PacketOrderNumber = ++Client.C2S_PacketOrderNumber;
		::strcpy_s(packet.Name, NAME_BUFFER_MAX, buff_name);

		SendEx(Client.Socket, (char*)&packet, packet.PacketLength);
	}

	void Send_C2S_Message(char* buff_message, UINT buff_message_length)
	{
		C2S_Message packet = {};

		packet.PacketLength = sizeof(HeadPacketInfo) + buff_message_length;
		packet.PacketType = PACKET__C2S_MESSAGE;
		packet.PacketOrderNumber = ++Client.C2S_PacketOrderNumber;
		::strcpy_s(packet.Message, MESSAGE_BUFFER_MAX, buff_message);

		SendEx(Client.Socket, (char*)&packet, packet.PacketLength);
	}

	void Send_C2S_Message_2(char* buff_message, UINT buff_message_length)
	{
		C2S_Message_2 packet = {};

		packet.PacketLength = sizeof(HeadPacketInfo) + buff_message_length;
		packet.PacketType = PACKET__C2S_MESSAGE_2;
		packet.PacketOrderNumber = ++Client.C2S_PacketOrderNumber;
		::strcpy_s(packet.Message, MESSAGE_BUFFER_MAX, buff_message);

		SendEx(Client.Socket, (char*)&packet, packet.PacketLength);
	}

	bool SendEx(SOCKET socket, char* packet, WORD packet_length)
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
				if (error == WSAEWOULDBLOCK) // 10035 - No package on buffer
				{
					if (++send_retry_count < send_retry_max)
					{
						::Sleep(1);
						continue;
					}
					printf("send failed, WSAEWOULDBLOCK, send_retry_count full, socket= %d, error: %d\n", socket, error);
					return false;
				}
				printf("send failed, socket= %d, error: %d\n", socket, error);
				return false;
			}
		}
		printf("send okay, send_length: %d, socket= %d\n", send_length, socket);
		return false;
	}

	bool RecvEx(SOCKET socket)
	{
		int recv_length = 0;
		char* buffer = (char*)&RecvPacket;

		const int recv_retry_max = 5000;
		int recv_retry_count = 0;

		while (RecvPacketLength > 0)
		{
			// Receive packet : packet header and length
			recv_length = ::recv(socket, &buffer[RecvPacketBufferIndex], RecvPacketLength, 0);
			if (recv_length > 0)
			{
				RecvPacketLength -= recv_length;
				RecvPacketBufferIndex += recv_length;
			}
			else if (recv_length == 0)
			{
				printf("#1 recv notice, client connection closed, socket= %d\n", socket);
				return false;
			}
			else
			{
				int error = ::WSAGetLastError();
				if (error == WSAEWOULDBLOCK) // 10035 - No package on buffer
				{
					return true;			// No data need to be receive
				}
				printf("#1 recv failed, socket= %d, error: %d\n", socket, error);
				return false;
			}
		}

		RecvPacketLength = RecvPacket.PacketLength - PACKET_LENGTH_SIZE;

		while (RecvPacketLength > 0)
		{
			// Receive packet : receive until packet is completely.
			recv_length = ::recv(socket, &buffer[RecvPacketBufferIndex], RecvPacketLength, 0);
			if (recv_length > 0)
			{
				RecvPacketLength -= recv_length;
				RecvPacketBufferIndex += recv_length;
			}
			else if (recv_length == 0)
			{
				printf("#2 recv notice, client connection closed, socket= %d\n", socket);
				ClearRecvPacketInfo();
				return false;
			}
			else
			{
				int error = ::WSAGetLastError();
				if (error == WSAEWOULDBLOCK) // 10035 - No package on buffer
				{
					if (++recv_retry_count < recv_retry_max)
					{
						::Sleep(1);
						continue;
					}
					printf("#2 recv failed, WSAEWOULDBLOCK, recv_retry_count full, socket= %d, error: %d\n", socket, error);
					ClearRecvPacketInfo();
					return false; // Socket may dis connect.
				}
				printf("#2 recv failed, socket= %d, error: %d\n", socket, error);
				ClearRecvPacketInfo();
				return false;
			}
		}

		if (RecvPacket.PacketOrderNumber == ++Client.S2C_PacketOrderNumber)
			printf("recv okay, socket= %d, packet_length= %d, [%d] buffer= %s\n", socket, RecvPacketBufferIndex, RecvPacket.PacketOrderNumber, RecvPacket.Buffer);
		else
			printf("recv okay, ERROR PacketOrderNumber : socket= %d, packet_length= %d, server[%d], client[%d], buffer= %s\n", socket, RecvPacketBufferIndex, RecvPacket.PacketOrderNumber, Client.S2C_PacketOrderNumber, RecvPacket.Buffer);

		ClearRecvPacketInfo();

		return true;
	}

	void ClearRecvPacketInfo()
	{
		RecvPacket = {};
		RecvPacketLength = PACKET_LENGTH_SIZE;
		RecvPacketBufferIndex = 0;
	}

	void Shutdown()
	{
		// shutdown a socket producure.
		// SD_SEND will be turn off
		int result = ::shutdown(Client.Socket, SD_SEND);
		if (result == SOCKET_ERROR)
		{
			printf("shutdown failed, error: %d\n", WSAGetLastError());
			return;
		}

		::Sleep(100); // Wait let client can disconnect correctly.

		char buff_message[PACKET_BUFFER_MAX] = { 0 };

		// Before ending receive length will be 0. Perfect shutdown.
		// Or -1 WSAEWOULDBLOCK is also OK.
		do
		{
			// recv for fragment packet and Disconnect Packet.
			result = ::recv(Client.Socket, buff_message, PACKET_BUFFER_MAX, 0);
			if (result > 0)
				printf("final recv okay, recv_length: %d\n", result);
			else if (result == 0)
				printf("final recv notice, client connection closed.\n");
			else
			{
				int error = ::WSAGetLastError();
				if (error == WSAEWOULDBLOCK) // 10035 - No more package in buffer
				{
					printf("final recv notice, client connection closed. - WSAEWOULDBLOCK\n");
				}
				else
				{
					printf("final recv failed, error: %d\n", error);
					break;
				}
			}

		} while (result > 0);
	}

	virtual void vOnRunTask(TaskNode* node)
	{
		switch (node->TaskNumber)
		{
		case TASK__ALIVE:
		{
			Send_C2S_Alive();
			printf("vOnRunTask   node->TaskNumber= %d, send alive packet.\n", node->TaskNumber);
			break;
		}
		}
	}

	void Release()
	{
		Shutdown();

		if (Client.Socket != INVALID_SOCKET)
			::closesocket(Client.Socket);
		Client.Socket = INVALID_SOCKET;

		WSACleanup();

		MY_RELEASE(TaskManager);
	}

	void Run()
	{
		Logger::Create();

		const UINT memory_pool_bytes_max = 100 * MB;
		MainMemoryPool::sMemoryPoolCreate(memory_pool_bytes_max);

		if (Create())
		{
			NameInput();
			ChatInput();
		}
		Release();

		MainMemoryPool::sMemoryPoolRelease();
		MemoryCounter::sShow_MemoryUseCount();
	}
};

int main()
{
	TCPClient client;
	client.Run();

	getc(stdin);
	return 0;
}