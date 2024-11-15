#include "server.h"
#include <conio.h>

LRESULT CALLBACK SERVER::wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    if (message == WM_CREATE) {
        SetWindowLongPtrW(hWnd, GWLP_USERDATA,
            reinterpret_cast<LONG_PTR>(reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams));
        return 0;
    }

    SERVER* server = reinterpret_cast<SERVER*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
    return server->handleWndProc(hWnd, message, wParam, lParam);
}

LRESULT SERVER::handleWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    if (message == WM_SOCKET) {
        if (wParam == serverSocket) processServerMsg();
        else processClientMsg(wParam, WSAGETSELECTEVENT(lParam), WSAGETSELECTERROR(lParam));
        return 0;
    }
    return DefWindowProcW(hWnd, message, wParam, lParam);
}

SERVER::SERVER() {
    WNDCLASS wc = {};
    wc.lpfnWndProc = SERVER::wndProc;
    wc.hInstance = GetModuleHandleW(NULL);
    wc.lpszClassName = L"ServerWindowClass";
    RegisterClass(&wc);

    hWnd = CreateWindowExW(0, L"ServerWindowClass", L"Server Window", 0,
        0, 0, 0, 0, HWND_MESSAGE, NULL, GetModuleHandleW(NULL), this);

    if (!hWnd) {
        throw std::runtime_error("Failed to create window");
    }

    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        throw std::runtime_error("Failed to initialize Winsock");
    }

    serverSocket = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);
    if (serverSocket == ERROR_SOCKET) {
        WSACleanup();
        throw std::runtime_error("Failed to create socket");
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(5555);
    if (inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr) <= 0) {
        closesocket(serverSocket);
        WSACleanup();
        throw std::runtime_error("Failed to set address");
    }

    if (bind(serverSocket, reinterpret_cast<SOCKADDR*>(&serverAddress), sizeof(serverAddress)) == SOCKET_ERROR) {
        closesocket(serverSocket);
        WSACleanup();
        throw std::runtime_error("Failed to bind socket");
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        closesocket(serverSocket);
        WSACleanup();
        throw std::runtime_error("Failed to listen on socket");
    }

    if (WSAAsyncSelect(serverSocket, hWnd, WM_SOCKET, FD_ACCEPT | FD_CLOSE) == SOCKET_ERROR) {
        closesocket(serverSocket);
        WSACleanup();
        throw std::runtime_error("Failed to set async select mode");
    }
}

SERVER::~SERVER() {
    for (auto& socket : clientSockets) {
        if (socket != ERROR_SOCKET) {
            closesocket(socket);
        }
    }
    closesocket(serverSocket);
    WSACleanup();
    DestroyWindow(hWnd);
}

void SERVER::processServerMsg() {
    SOCKET clientSocket = accept(serverSocket, NULL, NULL);
    if (clientSocket == ERROR_SOCKET) {
        std::cerr << "Accept failed with error: " << WSAGetLastError() << '\n';
        return;
    }

    int clientIndex = -1;
    for (int i = 0; i < clientSockets.size(); i++) {
        if (clientSockets[i] == 0) {
            clientIndex = i;
            break;
        }
    }

    if (clientIndex == -1) {
        std::cout << "Maximum clients reached. Connection rejected.\n";
        closesocket(clientSocket);
        return;
    }

    clientSockets[clientIndex] = clientSocket;
    std::cout << "Client " << clientIndex << " connected\n";

    if (WSAAsyncSelect(clientSocket, hWnd, WM_SOCKET, FD_READ | FD_CLOSE) == SOCKET_ERROR) {
        std::cerr << "Failed to set client events\n";
        closesocket(clientSocket);
        clientSockets[clientIndex] = ERROR_SOCKET;
        return;
    }
}

void SERVER::processClientMsg(SOCKET clientSocket, WPARAM event, LPARAM error) {
    int clientIndex = -1;
    for (int i = 0; i < clientSockets.size(); i++) {
        if (clientSockets[i] == clientSocket) {
            clientIndex = i;
            break;
        }
    }

    if (clientIndex == -1) return;

    if (event == FD_READ) {
        std::array<char, 512> buffer;
        int bytesRead = recv(clientSocket, buffer.data(), buffer.size() - 1, 0);

        if (bytesRead == SOCKET_ERROR) {
            std::cerr << "Recv failed with error: " << WSAGetLastError() << '\n';
            closesocket(clientSocket);
            clientSockets[clientIndex] = ERROR_SOCKET;
        }
        else if (bytesRead == 0) {
            std::cout << "Client " << clientIndex << " disconnected\n";
            closesocket(clientSocket);
            clientSockets[clientIndex] = ERROR_SOCKET;
        }
        else {
            buffer[bytesRead] = '\0';
            std::cout << "Received from client " << clientIndex << ": " << std::string(buffer.data()) << '\n';

            short amount = 0;
            char prevSymb = ' ';
            for (char ch : buffer) {
                if (ch != ' ' && prevSymb == ' ') amount++;
                prevSymb = ch;
            }

            std::cout << "Answer for client " << clientIndex << ": " << amount << '\n';
            send(clientSocket, std::to_string(amount).c_str(), std::to_string(amount).length(), 0);
        }
    }
    else if (event == FD_CLOSE) {
        std::cout << "Client " << clientIndex << " connection closed\n";
        closesocket(clientSocket);
        clientSockets[clientIndex] = ERROR_SOCKET;
    }
}

std::string SERVER::getAddr() {
    char ipStr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(serverAddress.sin_addr), ipStr, INET_ADDRSTRLEN);

    return std::string(ipStr) + ":" + std::to_string(ntohs(serverAddress.sin_port));
}

void SERVER::processMsgs() {
    while (running) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (_kbhit()) {
            char ch = _getch();
            if (ch == 'q' || ch == 'Q') {
                running = false;
            }
        }

        Sleep(10);
    }
}
