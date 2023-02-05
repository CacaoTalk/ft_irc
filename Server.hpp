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
# include <sys/errno.h>
# include <vector>

# include "User.hpp"
# include "Channel.hpp"
# include "Message.hpp"
# include "Command.hpp"
# include "Reply.hpp"

# define ERR_RETURN -1
# define CR '\r'
# define LF '\n'
# define SERVER_HOSTNAME "cacaotalk.42seoul.kr"


using namespace std;

class Server {
    private:
        int _fd;
        int _kq;
        int _port;
        string _password;
        map<int, User *> _allUser;
        map<string, Channel *> _allChannel;
        vector<struct kevent> _eventCheckList;
        struct kevent _waitingEvents[8];
        Command _command;

        Server(void);
        Server(const Server& server);
        Server& operator=(const Server& server);

        void initKqueue(void);
        void updateEvents(int socket, int16_t filter, uint16_t flags, uint32_t fflags, intptr_t data, void *udata);

        void acceptNewClient(void);
        void recvDataFromClient(const struct kevent& event);
        void sendDataToClient(const struct kevent& event);
        void handleEvent(const struct kevent& event);

        void handleMessageFromBuffer(User* user);
        size_t checkCmdBuffer(const User *user) const;

    public:
        Server(int port, string password);
        ~Server();

        const map<string, Channel *>& getAllChannel(void) const;

        User* findClientByNickname(const string& nickname) const;
        Channel* findChannelByName(const string& name) const;

        bool checkPassword(const string& password) const;
        Channel* addChannel(const string& name);
        void deleteChannel(const string& name);
        void disconnectClient(int clientFd);
        void run(void);
        void shutDown(const string& msg);
};

#endif
