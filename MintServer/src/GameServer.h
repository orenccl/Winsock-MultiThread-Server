#pragma once
#include "MTLibrary/TCPServer.h"

class GameServer : public TCPServer
{
public:
	void MainRun();

	virtual void vOnGameplayReceivePocket(TSNode< RecvPocketInfo >* pocket_node) override;
	
};