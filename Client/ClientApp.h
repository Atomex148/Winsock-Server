#pragma once
#define WIN32_LEAN_AND_MEAN
#define ERROR_SOCKET 0
#include <string>
#include <Windows.h>
#include <WinSock2.h>
#include <msclr/marshal_cppstd.h>

namespace ClientGUI {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	public ref class ClientApp : public System::Windows::Forms::Form
	{
	public:
		ClientApp(void)
		{
			InitializeComponent();

			pWsaData = new WSADATA;
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

	protected:
		~ClientApp()
		{
			if (components)
			{
				delete components;
			}
			closesocket(clientSocket);
			WSACleanup();
		}

	private:
		System::ComponentModel::Container ^components;
		System::Windows::Forms::RichTextBox^ numArrayInputTextBox1;
		System::Windows::Forms::Button^ sendButton;

		//client socks data
		WSADATA* pWsaData = nullptr;
		sockaddr_in* pServerAddr = nullptr;
		SOCKET clientSocket = ERROR_SOCKET;
	private: System::Windows::Forms::Button^ reconnectButton;
		   WSAEVENT* wsaEvent = new WSAEVENT[1]{ WSACreateEvent() };
		
#pragma region Windows Form Designer generated code
		/// <summary>
		/// “ребуемый метод дл€ поддержки конструктора Ч не измен€йте 
		/// содержимое этого метода с помощью редактора кода.
		/// </summary>
		void InitializeComponent(void)
		{
			this->numArrayInputTextBox1 = (gcnew System::Windows::Forms::RichTextBox());
			this->sendButton = (gcnew System::Windows::Forms::Button());
			this->reconnectButton = (gcnew System::Windows::Forms::Button());
			this->SuspendLayout();
			// 
			// numArrayInputTextBox1
			// 
			this->numArrayInputTextBox1->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
			this->numArrayInputTextBox1->Location = System::Drawing::Point(13, 13);
			this->numArrayInputTextBox1->MaxLength = 511;
			this->numArrayInputTextBox1->Name = L"numArrayInputTextBox1";
			this->numArrayInputTextBox1->Size = System::Drawing::Size(312, 75);
			this->numArrayInputTextBox1->TabIndex = 0;
			this->numArrayInputTextBox1->Text = L"";
			this->numArrayInputTextBox1->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &ClientApp::controlInput);
			// 
			// sendButton
			// 
			this->sendButton->Location = System::Drawing::Point(13, 95);
			this->sendButton->Name = L"sendButton";
			this->sendButton->Size = System::Drawing::Size(213, 23);
			this->sendButton->TabIndex = 1;
			this->sendButton->Text = L"Calculate amount of nums";
			this->sendButton->UseVisualStyleBackColor = true;
			this->sendButton->Click += gcnew System::EventHandler(this, &ClientApp::sendButton_Click);
			// 
			// reconnectButton
			// 
			this->reconnectButton->Location = System::Drawing::Point(233, 95);
			this->reconnectButton->Name = L"reconnectButton";
			this->reconnectButton->Size = System::Drawing::Size(92, 23);
			this->reconnectButton->TabIndex = 2;
			this->reconnectButton->Text = L"Reconnect";
			this->reconnectButton->UseVisualStyleBackColor = true;
			this->reconnectButton->Click += gcnew System::EventHandler(this, &ClientApp::reconnectButton_Click);
			// 
			// ClientApp
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(337, 127);
			this->Controls->Add(this->reconnectButton);
			this->Controls->Add(this->sendButton);
			this->Controls->Add(this->numArrayInputTextBox1);
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedSingle;
			this->MaximizeBox = false;
			this->Name = L"ClientApp";
			this->Text = L"ClientApp";
			this->ResumeLayout(false);

		}
#pragma endregion
		void controlInput(System::Object^ sender, System::Windows::Forms::KeyPressEventArgs^ e) {
			if ((e->KeyChar == static_cast<char>(Keys::Space) && (this->numArrayInputTextBox1->Text->Length == 0 || this->numArrayInputTextBox1->Text[this->numArrayInputTextBox1->Text->Length - 1] == ' '))) {
				e->Handled = true;
				return;
			}

			if (!Char::IsDigit(e->KeyChar) && e->KeyChar != static_cast<char>(Keys::Back) && e->KeyChar != static_cast<char>(Keys::Space)) {
				e->Handled = true;
			}
		}

		void sendButton_Click(System::Object^ sender, System::EventArgs^ e);
		void reconnectButton_Click(System::Object^ sender, System::EventArgs^ e);
	};
}
