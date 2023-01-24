#include "Server.hpp"

using namespace std;

int main(void) {
    try {
        Server ircServer;

        ircServer.run();    
    } catch(exception &e) {
        e.what();
    }

    return 0;
}