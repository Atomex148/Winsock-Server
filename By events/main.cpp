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

    server->processingMsgs();
	return 0;
}