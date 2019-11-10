#pragma once
#include "TCPServer.h"

class GameServer : public TCPServer
{
public:
	void Run();
};