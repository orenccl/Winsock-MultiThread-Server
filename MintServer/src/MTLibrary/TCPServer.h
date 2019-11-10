#pragma once
#define CLIENT_MAX     1000 // socket max

#include "MTLibrary/Type.h"
#include "MTLibrary/List/TDoublyList.hpp"

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
	TDoublyList< SocketContext > ClientList;
	bool    CreateSucceedFlag;
	bool    RunFlag;

	TCPServer();

	bool Create( WORD port );
	bool CreateWinsock( WORD port );
	void Accept();
	void Receive();
	bool ReceiveOne( SocketContext *node );
	void CloseSocket( SOCKET &socket );
	void Release();

	void Run();
};