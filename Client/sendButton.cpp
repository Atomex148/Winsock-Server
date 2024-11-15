#include "ClientApp.h"

void ClientGUI::ClientApp::sendButton_Click(System::Object^ sender, System::EventArgs^ e) {
	std::string nums = msclr::interop::marshal_as<std::string>(this->numArrayInputTextBox1->Text);
	if (nums.empty()) return;

	WSAEventSelect(clientSocket, wsaEvent[0], FD_READ);
	int iResult = send(clientSocket, nums.c_str(), nums.length(), 0);
	if (iResult == SOCKET_ERROR) {
		int errorCode = WSAGetLastError();
		std::wstring errorMessage = L"Error while sending message to server: " + std::to_wstring(errorCode);
		MessageBox::Show(gcnew String(errorMessage.c_str()), "Error", MessageBoxButtons::OK, MessageBoxIcon::Error);
		return;
	}

	short amount = 0;
	char prevSymb = ' ';
	for each (char ch in this->numArrayInputTextBox1->Text) {
		if (ch != ' ' && prevSymb == ' ') amount++;
		prevSymb = ch;
	}

	short respond = 0;
	while (respond != amount) {
		DWORD result = WSAWaitForMultipleEvents(1, wsaEvent, TRUE, 2500, FALSE);
		if (result == WSA_WAIT_FAILED || result == WSA_WAIT_TIMEOUT) {
			MessageBox::Show("Server not responding", "Error", MessageBoxButtons::OK, MessageBoxIcon::Error);
			return;
		}

		char buff[4];
		int bytesRead = recv(clientSocket, buff, 3, 0);
		if (bytesRead <= 0) {
			MessageBox::Show("Error receiving data", "Error", MessageBoxButtons::OK, MessageBoxIcon::Error);
			return;
		}

		buff[bytesRead] = '\0';
		respond = atoi(buff);
	}

	std::wstring resultStr = L"Amount of nums in array: " + std::to_wstring(respond);
	MessageBox::Show(gcnew String(resultStr.c_str()));
	WSAResetEvent(wsaEvent[0]);
}