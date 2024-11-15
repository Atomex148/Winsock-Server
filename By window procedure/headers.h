#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define WM_SOCKET (WM_USER + 1)
#define ERROR_SOCKET 0

#include <windows.h>
#include <WinSock2.h>
#include <ws2tcpip.h> 
#include <iostream>
#include <string>
#include <memory>
#include <array>
