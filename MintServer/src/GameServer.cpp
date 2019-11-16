#include "mtpch.h"
#include "GameServer.h"

void GameServer::MainRun()
{
	Logger::Create();
	TCPServer::Create(SERVER_PORT);

	TCPServer::MainRun();

	TCPServer::Release();
	MemoryCounter::sShow_MemoryUseCount();
}

void GameServer::vOnGameplayReceivePocket(TSNode< RecvPocketInfo >* pocket_node)
{
	// Send to all client
	AddSendList(SEND__ALL, pocket_node->Socket, pocket_node->Pocket, pocket_node->PocketLength);
}