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
        string _command;
        vector<string> _params;
        
        Message();
        Message(const Message& packet);
        Message& operator=(const Message& packet);

        void parse(const string& msg);

    public:
        Message(const string& msg);
        ~Message();

        static const string createReplyForm(const string& src, const string& cmd, const string& dstFirst, const string& dstSecond, const string& msg);

        vector<string> split(const string& str, const char delimeter);
        string getCommand() const;
        vector<string> getParams() const;
};


#endif
