#include "ClientApp.h"

void ClientGUI::ClientApp::reconnectButton_Click(System::Object^ sender, System::EventArgs^ e) {
	int iResult = WSAStartup(MAKEWORD(2, 2), pWsaData);
	if (iResult != 0) {
		MessageBox::Show("Cannot startup the WinSock", "Error", MessageBoxButtons::OK, MessageBoxIcon::Error);
		return;
	}

	clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (clientSocket == ERROR_SOCKET) {
		int errorCode = WSAGetLastError();
		std::wstring errorMessage = L"Error: " + std::to_wstring(errorCode);
		MessageBox::Show(gcnew System::String(errorMessage.c_str()), "Error", MessageBoxButtons::OK, MessageBoxIcon::Error);
		WSACleanup();
		return;
	}

	pServerAddr = new sockaddr_in;
	pServerAddr->sin_family = AF_INET;
	pServerAddr->sin_port = htons(5555);
	pServerAddr->sin_addr.s_addr = inet_addr("127.0.0.1");
	iResult = connect(clientSocket, reinterpret_cast<sockaddr*>(pServerAddr), sizeof(*pServerAddr));
	if (iResult == SOCKET_ERROR) {
		int errorCode = WSAGetLastError();
		std::wstring errorMessage = L"Error: " + std::to_wstring(errorCode);
		MessageBox::Show(gcnew System::String(errorMessage.c_str()), "Error", MessageBoxButtons::OK, MessageBoxIcon::Error);
		closesocket(clientSocket);
		WSACleanup();
		return;
	}
}