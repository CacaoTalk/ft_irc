#pragma once

#ifndef MESSAGE_HPP
# define MESSAGE_HPP
# include <string>
# include <vector>

using namespace std;

class Server;
class Message {
    private:
        int _fd;
        string _command;
        vector<string> _params;
        
        Message();
        Message(const Message& packet);
        Message& operator=(const Message& packet);

        vector<string> split(const string& str, const char delimeter);
        void parse(const string& msg);
        void cmdPrivmsg(Server& server);

    public:
        Message(int fd, const string& msg);
        ~Message();
        void runCommand(Server& server);
};

#endif