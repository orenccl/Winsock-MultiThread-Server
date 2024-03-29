#pragma once

/*! Precompile header */

/*! Window Library */
#include <WinSock2.h> // Winsock header must include before windows.h.
#include <ws2tcpip.h>
#pragma comment( lib, "winmm.lib" )
#pragma comment( lib, "WS2_32.lib" ) // winsock2.h
#include <windows.h>

/*! Standard Library */
#include <stdio.h>

/*! MintServer Header */
#include "MTLibrary/Type.h"


