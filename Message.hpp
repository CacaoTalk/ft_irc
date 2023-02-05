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
        string _prefix;
        string _command;
        vector<string> _params;
        
        Message(const Message& packet);
        Message& operator=(const Message& packet);

        void parse(const string& ircMsgFormStr);

    public:
        Message(void);
        Message(const string& ircMsgFormStr);
        ~Message();

        const string& getPrefix(void) const;
        const string& getCommand(void) const;
        const vector<string>& getParams(void) const;

        static vector<string> split(const string& str, const char delimeter);
        
        size_t paramSize(void) const;
        const string createReplyForm(void) const;
        Message& operator<<(const string param);
};

#endif
