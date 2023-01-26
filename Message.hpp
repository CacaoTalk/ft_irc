#pragma once

#ifndef MESSAGE_HPP
# define MESSAGE_HPP
# include <string>
# include <vector>

using namespace std;

class Server;
class User;
class Message {
    private:
        User *_user;
        string _command;
        vector<string> _params;
        
        Message();
        Message(const Message& packet);
        Message& operator=(const Message& packet);

        vector<string> split(const string& str, const char delimeter);
        void parse(const string& msg);
        void cmdPrivmsg(Server& server);
        void cmdJoin(Server& server);

    public:
        Message(User *user, const string& msg);
        ~Message();
        void runCommand(Server& server);
};

#endif
