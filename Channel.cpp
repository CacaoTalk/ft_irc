#include "Channel.hpp"
#include "Message.hpp"

Channel::Channel(const string& name): _name(name) {}

Channel::~Channel() { }

const string& Channel::getName(void) const {
    return _name;
}

const vector<string> Channel::getUserList(void) const {
    vector<string> userList;

    for (map<int, User *>::const_iterator it = _userList.begin(); it != _userList.end(); ++it) {
        string nickname = "";

        if (isUserOper(it->second->getFd())) nickname += '@';
        nickname += it->second->getNickname();
        userList.push_back(nickname);
    }
    return userList;
}

void Channel::addUser(int clientFd, User *user) {
    if (_userList.empty()) _operList.insert(clientFd);
    _userList.insert(make_pair(clientFd, user));
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

    if (_operList.empty()) {
       pair<int, User *> nextOper;

       nextOper = *_userList.begin();
       _operList.insert(nextOper.first);
       broadcast(Message() << ":" << clientName << "MODE" << getName() << "+o" << nextOper.second->getNickname());
    }
    return _userList.size();
}

User* Channel::findUser(const int clientFd) {
    map<int, User *>::iterator it;

    it = _userList.find(clientFd);
    if (it == _userList.end()) return NULL;
    return it->second;
}

bool Channel::isUserOper(int clientFd) const {
    set<int>::iterator it;

    return (_operList.find(clientFd) != _operList.end());
}

void Channel::broadcast(const Message& msg, int ignoreFd) {
    map<int, User *>::iterator it;

    for(it = _userList.begin(); it != _userList.end(); ++it) {
        if (it->first == ignoreFd) continue;

        it->second->addToReplyBuffer(msg.createReplyForm());
    }
}