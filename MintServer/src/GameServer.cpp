#include "mtpch.h"
#include "GameServer.h"

void GameServer::MainRun()
{
	TCPServer::Create(SERVER_PORT);
	Create();
	TCPServer::MainRun();
	Release();
	TCPServer::Release();
}

void GameServer::Create()
{
	PlayerInfoList.Create(CONNECTION_MAX);
}

void GameServer::Release()
{

}

void GameServer::vUserCreateClientNode(SOCKET_CLIENT_NODE* client_node)
{
	PLAYER_NODE* player_node = PlayerInfoList.GetNode();
	client_node->pUserData = player_node;
	PlayerInfoList.Link(player_node);
}

void GameServer::vUserFreeClientNode(SOCKET_CLIENT_NODE* client_node)
{
	PLAYER_NODE* player_node = (PLAYER_NODE*)client_node->pUserData;
	D_CHECK(player_node);
	PlayerInfoList.Unlink(player_node);
	PlayerInfoList.FreeNode(player_node);

	client_node->pUserData = NULL;
}

void GameServer::vOnGameplayReceivePacket(SOCKET_CLIENT_NODE* client_node, RECV_PACKET_NODE* packet_node, PacketBuffer* packet)
{
	PLAYER_NODE* player_node = (PLAYER_NODE*)client_node->pUserData;
	D_CHECK(player_node);

	switch (packet->PacketType)
	{
		case PACKET__C2S_NAME:
		{
			Tool::sMemcpy(player_node->Name, NAME_BUFFER_MAX, packet->Buffer, NAME_BUFFER_MAX);
			Logger::Log("socket= %d, set player_node->Name= %s", client_node->Socket, player_node->Name);
			break;
		}
		case PACKET__C2S_MESSAGE:
		{
			// Broadcast to all client

			S2C_Message s2c_packet = {};
			s2c_packet.PacketType = PACKET__S2C_MESSAGE;
			sprintf_s(s2c_packet.Message, MESSAGE_BUFFER_MAX - 1, "%s : %s", player_node->Name, packet->Buffer);
			s2c_packet.PacketLength = (WORD)(sizeof(HeadPacketInfo) + strlen(s2c_packet.Message));

			AddSendList(SEND__ALL, client_node, &s2c_packet);
			break;
		}
	}
}