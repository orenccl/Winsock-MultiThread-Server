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
		:TaskManager(NULL), RecvPacketLength(0), RecvPacketBufferIndex(0), Client({ 0 }), RecvPacket({0}) {} // class自動初始化為0
	
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

		// 初始化 winsock，使用2.2版本
		result = ::WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (result != 0)
		{
			printf("WSAStartup failed, error: %d\n", result);
			return false;
		}

		// 創建一個連線用client socket
		// AF_INET : 使用IPv4 協議
		// SOCK_STREAM : 一個序列化的連接導向位元流，可以做位元流傳輸。對應的protocol為TCP。
		// IPPROTO_TCP : 使用 TCP 協議。或也可填入NULL，表示使用系統預設的協議，即TCP協議。
		Client.Socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (Client.Socket == INVALID_SOCKET)
		{
			printf("socket failed, error: %d\n", ::WSAGetLastError());
			return false;
		}

		const char* ip = "127.0.0.1";             // 連到本機的server ip
		sockaddr_in addr = { 0 };
		addr.sin_family = AF_INET;                 // 使用IPv4 協議
		addr.sin_port = htons(SERVER_PORT);    // 設定port
		::inet_pton(AF_INET, ip, &addr.sin_addr); // 設定IP，client連線到server用

		result = ::connect(Client.Socket, (SOCKADDR*)&addr, sizeof(addr)); // connect連線到server...
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

		// 把ConnectSocket設為非阻塞模式（non-blocking）
		// FIONBIO : 設定或清除非阻塞模式
		unsigned long non_blocking_mode = 1; // 要啟用非阻塞模式, 所以設為1
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
		// 輸入暱稱字串

		int name_length = 0;

		while (1)
		{
			printf("name input : ");
			::fgets(Client.Name, NAME_BUFFER_MAX, stdin); // fgets()會等待從鍵盤輸入字串
			if (Client.Name[0] == '\n')
				continue;

			name_length = ::strlen(Client.Name); // 取得字串長度
			if (name_length > 0)
			{
				Client.Name[name_length - 1] = 0; // 因為使用fgets()，結尾會有一個 '\n' 符號，先刪除.
				name_length -= 1;
				break;
			}
		}

		Send_C2S_Name(Client.Name, name_length);
	}

	void ChatInput()
	{
		// 聊天主迴圈

		int  keyboard_state = 0, ch = 0;
		int  buff_message_length = 0;
		char buff_message[PACKET_BUFFER_MAX] = { 0 };

#define BACKSPCAE_KEY_CODE  8  // Backspace倒退鍵碼
#define ENTER_KEY_CODE     13  // Enter鍵碼

		printf("chat input... \n");


		// 使用winsock非阻塞模式，所以不會卡在某函式中(如：fgets, recv)，迴圈會不斷的執行和檢查狀態．
		while (1)
		{
			keyboard_state = ::_kbhit(); // 檢查是否有鍵盤按下，若有傳回1，若無傳回0
			if (keyboard_state > 0)
			{
				ch = ::_getch(); // 取得從鍵盤按下的一個鍵值
				printf("%c", ch);
				if (ch == ENTER_KEY_CODE) // 若按下Enter鍵...
				{
					printf("\n");
					if (::strcmp(buff_message, "exit") == 0) // 比較字串！若為 exit，則結束本程式
						break;

					if (buff_message_length > 0)
					{
						Send_C2S_Message(buff_message, buff_message_length);

						buff_message_length = 0;
						::memset(buff_message, 0, PACKET_BUFFER_MAX); // 把buff_message記憶體清除為0

						Client.pAliveNode->StartTime = 0; // 心跳包重新計時
					}
				}
				else if (ch == BACKSPCAE_KEY_CODE) // 若按下倒退鍵...
				{
					if (buff_message_length - 1 >= 0)
						buff_message[--buff_message_length] = 0;
				}
				else // 存入聊天訊息...
				{
					if (buff_message_length + 1 < PACKET_BUFFER_MAX)
						buff_message[buff_message_length++] = ch;
				}
			}

			if (RecvEx(Client.Socket) == false) // recv（非阻塞）：會不斷嘗試接收server端傳來的封包
				break;

			TaskManager->vRunTask();

			::Sleep(1); // 等待1毫秒
		}
	}
	
	void Send_C2S_Alive()
	{
		C2S_Alive packet = {};

		packet.PacketLength = sizeof(HeadPacketInfo);
		packet.PacketType = PACKET__C2S_ALIVE;
		packet.PacketOrderNumber = ++Client.C2S_PacketOrderNumber;

		SendEx(Client.Socket, (char*)&packet, packet.PacketLength); // send 傳送封包
	}
	
	void Send_C2S_Name(char* buff_name, UINT name_length)
	{
		C2S_Name packet = {};

		packet.PacketLength = sizeof(HeadPacketInfo) + name_length;
		packet.PacketType = PACKET__C2S_NAME;
		packet.PacketOrderNumber = ++Client.C2S_PacketOrderNumber;
		::strcpy_s(packet.Name, NAME_BUFFER_MAX, buff_name);

		SendEx(Client.Socket, (char*)&packet, packet.PacketLength); // send 傳送封包
	}

	void Send_C2S_Message(char* buff_message, UINT buff_message_length)
	{
		C2S_Message packet = {};

		packet.PacketLength = sizeof(HeadPacketInfo) + buff_message_length;
		packet.PacketType = PACKET__C2S_MESSAGE;
		packet.PacketOrderNumber = ++Client.C2S_PacketOrderNumber;
		::strcpy_s(packet.Message, MESSAGE_BUFFER_MAX, buff_message);

		SendEx(Client.Socket, (char*)&packet, packet.PacketLength); // send 傳送封包
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
			// 傳送封包
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
					printf("send failed, WSAEWOULDBLOCK, send_retry_count full, socket= %d, error: %d\n", socket, error);
					return false; // 此socket可能已經發生錯誤.
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
			// 接收封包：取得封包檔頭-總長度
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
				if (error == WSAEWOULDBLOCK) // 10035 - recv緩衝區內沒有封包了...
				{
					return true; // 此client是非阻塞模式，所以嘗試接收檔頭卻沒資料時，就立即返回．
				}
				printf("#1 recv failed, socket= %d, error: %d\n", socket, error);
				return false;
			}
		}

		RecvPacketLength = RecvPacket.PacketLength - PACKET_LENGTH_SIZE;

		while (RecvPacketLength > 0)
		{
			// 接收封包：取得一個完整的封包為止
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
				if (error == WSAEWOULDBLOCK) // 10035 - recv緩衝區內沒有封包了...
				{
					if (++recv_retry_count < recv_retry_max)
					{
						::Sleep(1);
						continue;
					}
					printf("#2 recv failed, WSAEWOULDBLOCK, recv_retry_count full, socket= %d, error: %d\n", socket, error);
					ClearRecvPacketInfo();
					return false; // 此socket可能無法再收到封包了，當作已經斷線.
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
		// shutdown優雅的關閉一個連接！會等待一小段時間，直到沒有更多封包要傳送時，才切斷連接！
		// SD_SEND：指定socket 接下來不能再呼叫傳送封包的函式。
		int result = ::shutdown(Client.Socket, SD_SEND);
		if (result == SOCKET_ERROR)
		{
			printf("shutdown failed, error: %d\n", WSAGetLastError());
			return;
		}

		::Sleep(100); // 等待100毫秒！讓被shutdown的client能夠有時間處理離線動作！

		char buff_message[PACKET_BUFFER_MAX] = { 0 };

		// 程式結束前的動作：若正確關閉後，recv會回傳0，即可優雅的結束程式．
		// 若回傳-1，並且錯誤碼是WSAEWOULDBLOCK時，仍是優雅的結束程式．
		do
		{
			// recv（非阻塞）：會嘗試接收server端傳來的殘存封包以及離線確認封包...(若有的話)
			result = ::recv(Client.Socket, buff_message, PACKET_BUFFER_MAX, 0);
			if (result > 0)
				printf("final recv okay, recv_length: %d\n", result);
			else if (result == 0)
				printf("final recv notice, client connection closed.\n");
			else
			{
				int error = ::WSAGetLastError();
				if (error == WSAEWOULDBLOCK) // 10035 沒有任何新封包時...
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
			::closesocket(Client.Socket); // 關閉socket
		Client.Socket = INVALID_SOCKET;

		WSACleanup(); // 釋放 winsock

		MY_RELEASE(TaskManager);
	}

	void Run()
	{
		Logger::Create();

		const UINT one_KB = 1024;
		const UINT one_MB = (one_KB * 1024);
		const UINT memory_pool_bytes_max = one_MB * 100;
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