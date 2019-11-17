#pragma once
#include "MTLibrary/TCPServer.h"

struct PlayerInfo
{
	UINT   ID;
	char   Name[NAME_BUFFER_MAX];
};
#define PLAYER_NODE  TDNode< PlayerInfo >

class GameServer : public TCPServer
{
	TMPDoublyList< PlayerInfo >  PlayerInfoList; // client PlayerInfo list ( For Gameplay_InThread use )

	void Create();
	void Release();

	virtual void vUserCreateClientNode(SOCKET_CLIENT_NODE* client_node);
	virtual void vUserFreeClientNode(SOCKET_CLIENT_NODE* client_node);

	virtual void vOnGameplayReceivePacket(SOCKET_CLIENT_NODE* client_node, RECV_PACKET_NODE* packet_node, PacketBuffer* packet);

public:
	void MainRun(); // Main thread entry point.
};