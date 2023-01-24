#pragma once

#ifndef MESSAGE_HPP
# define MESSAGE_HPP
# include <string>
# include <vector>

using namespace std;

class Message {
    private:
        string _commands;
        vector<string> _params;
        
        Message();
        Message(const Message& packet);
        Message& operator=(const Message& packet);

    public:
        Message(const string& msg);
        ~Message();
};

#endif
