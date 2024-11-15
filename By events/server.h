#pragma once
#include "headers.h"
#include "hiddenWindow.h"

class SERVER {
//VARS
	//status
	bool running = true;
	
	//sockets
	sockaddr_in serverAddress;
	WSADATA wsaData = { 0 };
	SOCKET serverSocket = { 0 };
	std::array<SOCKET, 12> clientSockets = { INVALID_SOCKET };

	//events
	std::array<WSAEVENT, 13> events;

//METHODS
	inline void processServerMsg();
	inline void processClientsMsgs(unsigned int index);
public:
	SERVER();
	~SERVER();

	void processingMsgs();
};