#include "mtpch.h"
#include "GameServer.h"

void GameServer::Run()
{
	Logger::Create();
	TCPServer::Create( SERVER_PORT );

	TCPServer::Run();

	TCPServer::Release();
	MemoryCounter::sShow_MemoryUseCount();
}