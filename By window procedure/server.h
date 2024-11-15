#pragma once
#include "headers.h"

class SERVER {
//VARS
    //running state
    bool running = true;

    //invisible window
    HWND hWnd = nullptr;
    MSG msg = { 0 };

    //socket things
    WSADATA wsaData;
    sockaddr_in serverAddress;
    SOCKET serverSocket;
    std::array<SOCKET, 12> clientSockets = {};

//METHODS
    void processServerMsg();
    void processClientMsg(SOCKET clientSocket, WPARAM wParam, LPARAM lParam);
    LRESULT handleWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
public:
//METHODS
    SERVER();
    ~SERVER();
    std::string getAddr();
    void processMsgs();
};
