#include "Channel.hpp"

Channel::Channel(const string& name): _name(name) {}

Channel::~Channel() { }

void Channel::addUser(int clientFd, User *user) {
    if (_userList.empty()) _operList.insert(clientFd);
    _userList.insert(pair<int, User *>(clientFd, user));
}

int Channel::deleteUser(int clientFd) {
    map<int, User *>::iterator it;
    string clientName;

    it = _userList.find(clientFd);
    if (it == _userList.end()) return _userList.size();
    
    clientName = it->second->getNickname();
    _userList.erase(clientFd);
    _operList.erase(clientFd);

    if (_userList.empty()) return 0;

    broadcast(clientName.append(" leave channel."));
    if (_operList.empty()) {
       pair<int, User *> nextOper;

       nextOper = *_userList.begin();
       _operList.insert(nextOper.first);
       broadcast(nextOper.second->getNickname().append(" is new channel operator."));
    }
    return _userList.size();
}

bool Channel::isUserOper(int clientFd) {
    set<int>::iterator it;

    if (_operList.find(clientFd) != _operList.end())
        return true;
    else
        return false;
}

void Channel::broadcast(const string& msg, int ignoreFd) {
    map<int, User *>::iterator it;

    for(it = _userList.begin(); it != _userList.end(); ++it) {
        if (it->first == ignoreFd) continue;

        it->second->addToReplyBuffer(msg);
    }
}