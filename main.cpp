#include <iostream>
#include "Server.hpp"

using namespace std;

int validatePort(char *portString) {
    char *pEnd;
    long port = strtol(portString, &pEnd, 10);
    
    if (errno == ERANGE || *pEnd != '\0' || port <= 0 || port > 65535) {
        cerr << "Invalid port!!\n";
        exit(EXIT_FAILURE);
    }
    return port;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        cerr << "Usage: ./server <port> <password>\n";
        exit(EXIT_FAILURE);
    }

    try {
        int port = validatePort(argv[1]);
        Server ircServer(port, argv[2]);

        ircServer.run();    
    } catch(exception &e) {
        e.what();
    }

    return 0;
}