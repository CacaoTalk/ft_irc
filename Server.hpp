#pragma once

#ifndef SERVER_HPP
# define SERVER_HPP

# include <string>
# include <map>
# include <exception>
# include <sys/types.h>
# include <sys/event.h>
# include <sys/time.h>
# include <sys/socket.h>
# include <arpa/inet.h>
# include <unistd.h>
# include <fcntl.h>
# include <vector>

# include "User.hpp"
# include "Channel.hpp"

using namespace std;

class Server {
    private:
        int _fd;
        int _kq;
        map<int, User *> _allUser;
        map<string, Channel *> _allChannel;
        vector<struct kevent> _eventCheckList;
        struct kevent _waitingEvents[8];

        Server(const Server& server);
        Server& operator=(const Server& server);

        void disconnectClient(int clientFd);
        void initKqueue();
        void updateEvents(int socket, int16_t filter, uint16_t flags, uint32_t fflags, intptr_t data, void *udata);
        void addChannel(const string& name);
        void deleteChannel(const string& name);

        void acceptNewClient(void);
        void readDataFromClient(const struct kevent& event);
        void sendDataToClient(const struct kevent& event);
        void handleEvent(const struct kevent& event);

        User* findClientByNickname(string nickname);
    public:
        Server(void);
        ~Server();
        void run();
        void shutDown(const string& msg);
};

#endif
