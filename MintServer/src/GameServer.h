#pragma once
#include "MTLibrary/TCPServer.h"

class GameServer : public TCPServer
{
public:
	void MainRun();

	virtual void vOnGameplayReceivePacket(TSNode< RecvPacketInfo >* pocket_node) override;
};