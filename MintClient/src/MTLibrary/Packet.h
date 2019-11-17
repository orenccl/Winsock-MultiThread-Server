#pragma once
#include <windows.h>

#define PACKET_LENGTH_TYPE   WORD
#define PACKET_LENGTH_SIZE   LENGTH_2
#define PACKET_BUFFER_MAX    512

enum _ACTION_TYPE_
{
	ACTION__CREATE_CLIENT = 1,
	ACTION__RELEASE_CLIENT,
	ACTION__PACKET,
};

struct RecvPacketInfo
{
	BYTE    ActionType; // _ACTION_TYPE_
	SOCKET Socket;
	WORD   PacketLength;
	char   Packet[PACKET_BUFFER_MAX];
};

#define RECV_PACKET_NODE  TSNode< RecvPacketInfo >

enum _SEND_TYPE_
{
	SEND__ALL = 1,
	SEND__SELF,
	SEND__NON_SELF,
	SEND__GROUP_ALL,
	SEND__GROUP_SELF,
	SEND__GROUP_NON_SELF,
};

#define SEND_PACKET_NODE  TSNode< SendPacketInfo >

struct SendPacketInfo
{
	BYTE   SendType; // _SEND_TYPE_
	SOCKET Socket;
	WORD   PacketLength;
	char   Packet[PACKET_BUFFER_MAX];
};

#define NAME_BUFFER_MAX      16
#define MESSAGE_BUFFER_MAX   128

struct HeadPacketInfo
{
	WORD PacketLength;
	WORD PacketType; // _PACKET_TYPE_
	WORD PacketOrderNumber;
};

struct PacketBuffer : HeadPacketInfo
{
	char Buffer[PACKET_BUFFER_MAX];
};

enum _PACKET_TYPE_
{
	PACKET__C2S_ALIVE = 1,
	PACKET__C2S_NAME,
	PACKET__C2S_MESSAGE,
	PACKET__S2C_MESSAGE,
};


struct C2S_Alive : HeadPacketInfo
{
};

struct C2S_Name : HeadPacketInfo
{
	char Name[NAME_BUFFER_MAX];
};

struct C2S_Message : HeadPacketInfo
{
	char Message[MESSAGE_BUFFER_MAX];
};

struct S2C_Message : C2S_Message
{

};