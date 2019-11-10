#include "GameServer.h"

void GameServer::Run()
{
	TCPServer::Create( SERVER_PORT );
	
	TCPServer::Run();

	TCPServer::Release();
}