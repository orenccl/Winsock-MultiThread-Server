#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdio.h>

#define SERVER_PORT   27015

struct SocketContext
{
	SOCKET  Socket;
	void   *pUserData;
};

class TCPServer
{
	SOCKET  ListenSocket;
	SocketContext Client;
	bool    CreateSucceedFlag;
	bool    RunFlag;
public :
	TCPServer();

	bool Create( WORD port );
	bool CreateWinsock( WORD port );
	bool Accept();
	void Receive();
	void CloseSocket( SOCKET &socket );
	void Release();

	void Run();
};