#pragma once

#define FD_SETSIZE        1000 // ­Modify Windos define fd_set structure, socket array max 1000
#define CONNECTION_MAX    FD_SETSIZE
#define ACCEPT_LIST_MAX   1000
#define RECEIVE_LIST_MAX  1000
#define SEND_LIST_MAX     1000

#include "MTLibrary/MemoryPool.h"
#include "MTLibrary/LockList.hpp"
#include "MTLibrary/Thread.h"
#include "MTLibrary/Packet.h"
#include "MTLibrary/Task.h"
#include "MTLibrary/Tool.h"

#define SERVER_PORT   27015

struct SocketInfo
{
	SOCKET  Socket;
	char    IP[LENGTH_16];
};

struct SocketContext
{
	SOCKET  Socket;
	char    IP[LENGTH_16];
	void*	pUserData;
};

#define SOCKET_NODE  TDNode< SocketInfo >

struct SocketCache
{
	UINT	SocketTableUseCount;
	SOCKET	SocketTable[CONNECTION_MAX];
};

struct TimerInfo
{
	INT64  StartTime;
	INT64  EndTime;
};

struct SocketClient
{
	SOCKET		Socket;
	char		IP[LENGTH_16];
	WORD		S2C_PacketOrderNumber;
	WORD		C2S_PacketOrderNumber;
	TimerInfo	AliveTimeout;
	bool		AliveTimeoutFlag;
	void*		pUserData; // PlayerInfo
};
#define SOCKET_CLIENT_NODE  TDNode< SocketClient >

class TCPServer
{
	TMPDoublyLockList< SocketInfo >      AcceptList;		// When there is a new connection, Accept_InThread put it into AcceptList, List to be initialize in Select_InMainThread.
	TMPDoublyList< SocketInfo >          SocketRecvList;    // Normal socket list ( For Select_InMainThread use )
	TMPDoublyLockList< SocketInfo >      SocketSendList;    // Normal socket list ( For Select_InMainThread use )
	SocketCache                          SocketSendCache;   // Socket cache table ( For Select_InMainThread use )
	TMPSinglyLockList< RecvPacketInfo >  PacketRecvList;    // When received pocket, Select_InMainThread put it into PacketRecvList, List to be receive in Gameplay_InThread.
	TMPSinglyLockList< SendPacketInfo >  PacketSendList;    // List to be send in Send_InThread.
	TMPDoublyList< SocketClient >		 ClientList;		// client SocketClient list ( For Gameplay_InThread use )

	SOCKET				ListenSocket;
	bool				CreateSucceedFlag;                   
	bool				RunFlag;                             

	CAcceptThread		AcceptThread;
	CThread				GameplayThread;                      
	CThread				SendThread;                          
	static UINT WINAPI sAcceptThreadProc(LPVOID param);
	static UINT WINAPI sGameplayThreadProc(LPVOID param);
	static UINT WINAPI sSendThreadProc(LPVOID param);

	void Select_InMainThread(); // MainThread execute select to detect new connect event or pocket event
	void Accept_InThread();     // sAcceptThreadProc deal with socket connect
	void Gameplay_InThread();   // sGameplayThreadProc deal with logical tasks.
	void Send_InThread();       // sSendThreadProc deal with TCP pocket.

	bool CreateWinsock(WORD port);
	void SetTCPNoDelay(SOCKET socket);					// Set TCP Nagle's algorithm off ( It will speed up package transmition )
	void AcceptListCreateClientNode();
	void RecvFreeClientNode(SOCKET_NODE*& del_node);	// When receive disconnect event, free clientnode into freepool.
	bool ReceiveEx(SOCKET socket);				// Deal with sticky package.
	bool SendEx(SOCKET socket, char* packet, WORD packet_length); // Deal with send incompletely.
	void CloseSocket(SOCKET& socket);
	void ServerExit();
	void ShutdownAndCloseSocket();						// Shutdown and close all socket.
	void CheckClientAlive();

	void NoticeGameplayThreadCreateClientNode(SOCKET_NODE* accept_node);	// Select_InMainThread inform GameplayThread to initialize client.
	void NoticeGameplayThreadFreeClientNode(SOCKET socket);
	void CreateClientNode(RECV_PACKET_NODE* packet_node);					// Initialize client player
	void FreeClientNode(RECV_PACKET_NODE* packet_node);
	virtual void vUserCreateClientNode(SOCKET_CLIENT_NODE* client_node) {}  // Child class initialize player
	virtual void vUserFreeClientNode(SOCKET_CLIENT_NODE* client_node) {}

	SOCKET_CLIENT_NODE* GetSocketClientNode(SOCKET socket);
	virtual void vOnGameplayReceivePacket(SOCKET_CLIENT_NODE* client_node, RECV_PACKET_NODE* packet_node, PacketBuffer* packet) {} // Logical tasks after received.

protected:
	TCPServer() {}

	void AddSendList(_SEND_TYPE_ send_type, SOCKET_CLIENT_NODE* client_node, HeadPacketInfo* packet);

	bool Create(WORD port);
	void MainRun(); // Main thread entry point
	void Release();
};