#include "ClientApp.h"
using namespace System;
using namespace System::Windows::Forms;

[STAThread]
int main(array<String^>^ args) {
    Application::EnableVisualStyles();
    Application::SetCompatibleTextRenderingDefault(false);
    ClientGUI::ClientApp app;
    Application::Run(%app);
    return 0;
}