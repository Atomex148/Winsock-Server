#include "headers.h"
#include "server.h"

int main() {
    std::unique_ptr<SERVER> server;
    try {
	    server = std::make_unique<SERVER>();
    }
    catch (const std::runtime_error& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }

    std::cout << "Server initilized at " << server->getAddr() << "\nPress Q to stop server\n";
    server->processMsgs();
	return 0;
}