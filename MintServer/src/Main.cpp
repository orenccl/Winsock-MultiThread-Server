#include "mtpch.h"
#include "GameServer.h"

int main()
{
	GameServer server;
	
	server.MainRun();

	getc(stdin);
	return 0;
}