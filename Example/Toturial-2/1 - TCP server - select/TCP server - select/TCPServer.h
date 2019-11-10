#pragma once
#include <winsock2.h>
#include <windows.h>

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
	SocketContext Client;
	bool    CreateSucceedFlag;
	bool    RunFlag;

	TCPServer();

	bool Create( WORD port );
	bool CreateWinsock( WORD port );
	void Select();
	void Accept();
	bool Receive();
	void CloseSocket( SOCKET &socket );
	void ShutdownAndCloseSocket();
	void Release();

	void Run();
};