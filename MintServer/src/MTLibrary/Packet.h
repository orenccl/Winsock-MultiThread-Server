#pragma once
#include <windows.h>

#define PACKET_BUFFER_MAX    512

struct RecvPacketInfo
{
	SOCKET Socket;
	WORD   PacketLength;
	char   Packet[PACKET_BUFFER_MAX];
};

enum _SEND_TYPE_
{
	SEND__ALL = 0,
	SEND__SELF,
	SEND__NON_SELF,
};

struct SendPacketInfo
{
	BYTE   SendType; // _SEND_TYPE_
	SOCKET Socket;
	WORD   PacketLength;
	char   Packet[PACKET_BUFFER_MAX];
};

#define NAME_BUFFER_MAX      16
#define MESSAGE_BUFFER_MAX   128
#define BUFFER_MAX           512

struct HeadPacketInfo
{
	BYTE Number;
	char Name[NAME_BUFFER_MAX];
};

struct PacketInfo : HeadPacketInfo
{
	char Message[MESSAGE_BUFFER_MAX];
};