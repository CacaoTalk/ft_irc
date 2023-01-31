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
# include "Message.hpp"

# define CR 13
# define LF 10
# define SERVER_HOSTNAME "cacaotalk.42seoul.kr"

// NUMERIC REPLIES
# define RPL_WELCOME "001"

# define RPL_NAMREPLY "353"
# define RPL_ENDOFNAMES "366"
# define RPL_ENDOFNAMES_MSG ":End of /NAMES list."

# define ERR_UNKNOWNERROR "400"
# define ERR_NOSUCHNICK_MSG ":No such nick/channel"
# define ERR_NOSUCHNICK "401"
# define ERR_NOSUCHSERVER "402"
# define ERR_NOSUCHCHANNEL "403"
# define ERR_NOSUCHCHANNEL_MSG ":No such channel"
# define ERR_CANNOTSENDTOCHAN "404"
# define ERR_TOOMANYCHANNELS "405"
# define ERR_NOORIGIN "409"
# define ERR_NOORIGIN_MSG ":No origin specified"
# define ERR_NORECIPIENT "411"
# define ERR_NORECIPIENT_MSG ":No recipient given"
# define ERR_NOTEXTTOSEND "412"
# define ERR_NOTEXTTOSEND_MSG ":No text to send"

# define ERR_NONICKNAMEGIVEN "431"
# define ERR_NONICKNAMEGIVEN_MSG ":No nickname given"
# define ERR_ERRONEUSNICKNAME "432"
# define ERR_NICKNAMEINUSE "433"
# define ERR_NICKNAMEINUSE_MSG ":Nickname is already in use"

# define ERR_USERNOTINCHANNEL "441"
# define ERR_USERNOTINCHANNEL_MSG ":They aren't on that channel"
# define ERR_NOTONCHANNEL "442"
# define ERR_NOTONCHANNEL_MSG ":You're not on that channel"
# define ERR_USERONCHANNEL "443"

# define ERR_NEEDMOREPARAMS "461"
# define ERR_NEEDMOREPARAMS_MSG ":Not enough parameters"
# define ERR_ALREADYREGISTERED "462"
# define ERR_ALREADYREGISTERED_MSG ":You may not reregister"
# define ERR_PASSWDMISMATCH "464"
# define ERR_PASSWDMISMATCH_MSG ":Password incorrect"

# define ERR_CHANNELISFULL "471"
# define ERR_CHANOPRIVSNEEDED "482"
# define ERR_CHANOPRIVSNEEDED_MSG ":You're not channel operator"

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

        Server(void);
        Server(const Server& server);
        Server& operator=(const Server& server);

        void disconnectClient(int clientFd);
        void initKqueue();
        void updateEvents(int socket, int16_t filter, uint16_t flags, uint32_t fflags, intptr_t data, void *udata);
        Channel* addChannel(const string& name);
        void deleteChannel(const string& name);

        void acceptNewClient(void);
        void readDataFromClient(const struct kevent& event);
        void sendDataToClient(const struct kevent& event);
        void handleEvent(const struct kevent& event);

        void handleMessageFromBuffer(User* user);
        size_t checkCmdBuffer(const User *user);

        User* findClientByNickname(const string& nickname);
        Channel* findChannelByName(const string& name);

        void runCommand(User *user, Message& msg);
        void cmdPrivmsg(User* user, Message& msg);
        void cmdJoin(User* user, Message& msg);
        void cmdPart(User* user, Message& msg);
        void cmdPass(User *user, Message& msg);
        void cmdNick(User *user, Message& msg);
        void cmdUser(User *user, Message& msg);
        void cmdPing(User *user, Message& msg);
        void cmdQuit(User *user, Message& msg);
        void cmdKick(User *user, Message& msg);
        void cmdNotice(User *user, Message& msg);

        friend class Message;
    public:
        Server(int port, string password);
        ~Server();
        void run();
        void shutDown(const string& msg);
};

#endif
