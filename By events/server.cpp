#include "server.h"
#include  <conio.h>

SERVER::SERVER() {
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        std::cerr << "WSAStartup failed with error: " << iResult << '\n';
        throw std::runtime_error("Failed to initialize Winsock");
    }

    serverSocket = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);
    if (serverSocket == ERROR_SOCKET) {
        std::cerr << "Socket init failed with error: " << WSAGetLastError() << '\n';
        throw std::runtime_error("Failed to initialize socket");
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(5555);
    if (inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr) <= 0) {
        std::cerr << "Invalid IP address format or inet_pton failed.\n";
        throw std::runtime_error("Failed to dedicate an address");
    }

    if (bind(serverSocket, reinterpret_cast<SOCKADDR*>(&serverAddress), sizeof(serverAddress)) == SOCKET_ERROR) {
        std::cerr << "Bind failed with error: " << WSAGetLastError() << '\n';
        throw std::runtime_error("Failed to bind socket");
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed with error: " << WSAGetLastError() << '\n';
        throw std::runtime_error("Failed to listen on socket");
    }

    events[0] = WSACreateEvent();
    if (events[0] == WSA_INVALID_EVENT) {
        std::cerr << "WSACreateEvent failed with error: " << WSAGetLastError() << '\n';
        throw std::runtime_error("Failed to create event");
    }

    if (WSAEventSelect(serverSocket, events[0], FD_ACCEPT | FD_READ | FD_CLOSE) == SOCKET_ERROR) {
        std::cerr << "WSAEventSelect failed with error: " << WSAGetLastError() << '\n';
        throw std::runtime_error("Failed to set event socket mode");
    }
}

SERVER::~SERVER() {
    for (auto& socket : clientSockets) {
        if (socket != ERROR_SOCKET) {
            closesocket(socket);
        }
    }
    closesocket(serverSocket);

    for (int i = 1; i < events.size(); i++) {
        WSACloseEvent(events[i]);
    }
    WSACleanup();
}

void SERVER::processServerMsg() {
    WSANETWORKEVENTS networkEvents;
    if (WSAEnumNetworkEvents(serverSocket, events[0], &networkEvents) == SOCKET_ERROR) {
        std::cerr << "WSAEnumNetworkEvents failed with error: " << WSAGetLastError() << '\n';
        return;
    }

    if (networkEvents.lNetworkEvents & FD_ACCEPT) {
        int clientIndex = -1;
        for (int i = 0; i < clientSockets.size(); i++) {
            if (clientSockets[i] == 0) {
                clientIndex = i;
                break;
            }
        }

        if (clientIndex == -1) {
            std::cout << "Maximum clients reached. Connection rejected.\n";
            SOCKET tempSocket = accept(serverSocket, NULL, NULL);
            closesocket(tempSocket);
            return;
        }

        clientSockets[clientIndex] = accept(serverSocket, NULL, NULL);
        if (clientSockets[clientIndex] == ERROR_SOCKET) {
            std::cerr << "Accept failed with error: " << WSAGetLastError() << '\n';
            return;
        }

        std::cout << "Client " << clientIndex << " connected\n";

        if (WSAEventSelect(clientSockets[clientIndex], events[clientIndex + 1], FD_READ | FD_CLOSE) == SOCKET_ERROR) {
            std::cerr << "Failed to set client events\n";
            closesocket(clientSockets[clientIndex]);
            clientSockets[clientIndex] = ERROR_SOCKET;
            return;
        }
    }
}

void SERVER::processClientsMsgs(unsigned int index) {
    WSANETWORKEVENTS networkEvents;
    std::array<char, 512> buffer;

    int clientIndex = index - 1;
    if (clientIndex >= 0 && clientIndex < clientSockets.size() &&
        clientSockets[clientIndex] != ERROR_SOCKET) {

        if (WSAEnumNetworkEvents(clientSockets[clientIndex], events[index], &networkEvents) == SOCKET_ERROR) {
            std::cerr << "WSAEnumNetworkEvents failed for client with error: " << WSAGetLastError() << '\n';
            return;
        }

        if (networkEvents.lNetworkEvents & FD_READ) {
            int bytesRead = recv(clientSockets[clientIndex], buffer.data(), buffer.size() - 1, 0);
            if (bytesRead == SOCKET_ERROR) {
                std::cerr << "Recv failed with error: " << WSAGetLastError() << '\n';
                closesocket(clientSockets[clientIndex]);
                clientSockets[clientIndex] = ERROR_SOCKET;
            }
            else if (bytesRead == 0) {
                std::cout << "Client " << clientIndex << " disconnected\n";
                closesocket(clientSockets[clientIndex]);
                clientSockets[clientIndex] = ERROR_SOCKET;
            }
            else {
                buffer[bytesRead] = '\0';
                std::cout << "Recieved from client " << clientIndex << ": " << std::string(buffer.data()) << '\n';
                short amount = 0;
                char prevSymb = ' ';
                for (char ch : buffer) {
                    if (ch != ' ' && prevSymb == ' ') amount++;
                    prevSymb = ch;
                }
                
                std::cout << "Answer for client " << clientIndex << ": " << amount << '\n';
                send(clientSockets[clientIndex], std::to_string(amount).c_str(), std::to_string(amount).length(), 0);
            }
        }

        if (networkEvents.lNetworkEvents & FD_CLOSE) {
            std::cout << "Client " << clientIndex << " connection closed\n";
            closesocket(clientSockets[clientIndex]);
            clientSockets[clientIndex] = ERROR_SOCKET;
        }
    }
}

void SERVER::processingMsgs() {
    for (int i = 1; i < events.size(); i++) {
        events[i] = WSACreateEvent();
        if (events[i] == WSA_INVALID_EVENT) {
            std::cerr << "Failed to create event for client " << i << '\n';
            for (int j = 1; j < i; j++) {
                WSACloseEvent(events[j]);
            }
            return;
        }
    }

    while (running) {
        DWORD dwWaitResult = WSAWaitForMultipleEvents(events.size(), events.data(), FALSE, 100, FALSE);

        if (_kbhit()) {
            char ch = _getch();
            if (ch == 'q' || ch == 'Q') {
                running = false;
            }
        }

        if (dwWaitResult == WSA_WAIT_FAILED) {
            std::cerr << "WSAWaitForMultipleEvents failed with error: " << WSAGetLastError() << '\n';
            break;
        }

        if (dwWaitResult == WSA_WAIT_TIMEOUT) continue;

        int index = dwWaitResult - WSA_WAIT_EVENT_0;
        if (index == 0) processServerMsg();
        else processClientsMsgs(index);
    }
}