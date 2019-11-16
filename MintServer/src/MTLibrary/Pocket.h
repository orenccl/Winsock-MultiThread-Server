#pragma once
#include <windows.h>

#define POCKET_BUFFER_MAX    512

struct RecvPocketInfo
{
	SOCKET Socket;
	WORD   PocketLength;
	char   Pocket[POCKET_BUFFER_MAX];
};

enum _SEND_TYPE_
{
	SEND__ALL = 0,
	SEND__SELF,
	SEND__NON_SELF,
};

struct SendPocketInfo
{
	BYTE   SendType; // _SEND_TYPE_
	SOCKET Socket;
	WORD   PocketLength;
	char   Pocket[POCKET_BUFFER_MAX];
};

#define NAME_BUFFER_MAX      16
#define MESSAGE_BUFFER_MAX   128
#define BUFFER_MAX           512

struct HeadPocketInfo
{
	BYTE Number;
	char Name[NAME_BUFFER_MAX];
};

struct PocketInfo : HeadPocketInfo
{
	char Message[MESSAGE_BUFFER_MAX];
};