#pragma once
#include "MTLibrary/TCPServer.h"

class GameServer : public TCPServer
{
public:
	void Run();
};