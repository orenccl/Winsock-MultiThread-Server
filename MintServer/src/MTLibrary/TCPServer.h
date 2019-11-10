#pragma once
#define FD_SETSIZE     1000 // socket max

#include "MTLibrary/List/TMPDoublyList.hpp"

#define SERVER_PORT   27015

struct SocketContext
{
	SOCKET  Socket;
	void   *pUserData;
};

class TCPServer
{
protected :
	SOCKET  ListenSocket;
	TMPDoublyList< SocketContext > ClientList;
	bool    CreateSucceedFlag;
	bool    RunFlag;

	TCPServer();

	bool Create( WORD port );
	bool CreateWinsock( WORD port );
	void Select();
	void Accept();
	bool Receive( SocketContext* node );
	void CloseSocket( SOCKET &socket );
	void ShutdownAndCloseSocket();
	void Release();

	void Run();
};