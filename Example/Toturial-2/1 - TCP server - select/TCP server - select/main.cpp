
#include "GameServer.h"
#include <stdio.h>

//=============================================================================================================================================================
int main()
{
	GameServer server;
	server.Run();

	getc( stdin );
	return 0;
}