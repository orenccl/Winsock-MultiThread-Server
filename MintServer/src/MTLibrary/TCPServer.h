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
	void* pUserData;
};

class TCPServer
{
	SOCKET      ListenSocket;
	TMPSinglyLockList< SocketInfo >      AcceptList;  // When there is a new connection, Accept_InThread put it into AcceptList, List to be initialize in Select_InMainThread.
	TMPDoublyList< SocketContext >       ClientList;
	TMPDoublyLockList< SocketInfo >      SocketList;
	TMPSinglyLockList< RecvPacketInfo >  ReceiveList; // When received pocket, main thread put it into ReceiveList, List to be receive in Gameplay_InThread.
	TMPSinglyLockList< SendPacketInfo >  SendList;    // List to be send in Send_InThread.
	bool        CreateSucceedFlag;                   
	bool        RunFlag;                             

	CThreadEx   AcceptThread;                        
	CThread     GameplayThread;                      
	CThread     SendThread;                          
	static UINT WINAPI sAcceptThreadProc(LPVOID param);
	static UINT WINAPI sGameplayThreadProc(LPVOID param);
	static UINT WINAPI sSendThreadProc(LPVOID param);

	bool CreateWinsock(WORD port);
	bool Receive(SocketContext* node);
	bool Send(SOCKET socket, const char* pocket, WORD pocket_length);
	void CloseSocket(SOCKET& socket);
	void ShutdownAndCloseSocket();
	
	void CreateClientNode(TSNode< SocketInfo >* accept_node);
	void FreeClientNode(TDNode< SocketContext >*& client_node);
	
	void Select_InMainThread(); // MainThread execute select to detect new connect event or pocket event
	void Accept_InThread();     // sAcceptThreadProc deal with socket connect
	void Gameplay_InThread();   // sGameplayThreadProc deal with logical tasks.
	void Send_InThread();       // sSendThreadProc deal with TCP pocket.
protected:
	TCPServer();

	bool Create(WORD port);
	void Release();
	void AddSendList(_SEND_TYPE_ send_type, SOCKET socket, char* pocket, WORD pocket_length);
	virtual void vOnGameplayReceivePacket(TSNode< RecvPacketInfo >* pocket_node) {} // Could be modify by inheritance Class.
	void MainRun(); // Main thread entry point
};