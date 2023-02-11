#pragma once

#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include <map>
# include <set>
# include <vector>

# include "Bot.hpp"
# include "CommonValue.hpp"

using namespace std;

class User;
class Message;

class Channel {
    private:
		string _name;
		map<int, User *> _userList;
		set<int> _operList;
        Bot _bot;

        Channel(void);
        Channel(const Channel& channel);
        Channel& operator=(const Channel& channel);

    public:
        Channel(const string& name);
        ~Channel();

        const string& getName(void) const;
        const vector<string> getUserList(void) const;

        void addUser(int clientFd, User *user);
        int deleteUser(int clientFd);
        User* findUser(const int clientFd);
        User* findUser(const string& nickname);
        bool isUserOper(int clientFd) const;
        void broadcast(const Message& msg, int ignoreFd = UNDEFINED_FD) const;

        void executeBot(const string& msgContent);
};

#endif
