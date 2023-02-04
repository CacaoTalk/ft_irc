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
    string clientSource;

    it = _userList.find(clientFd);
    if (it == _userList.end()) return _userList.size();
    
    clientSource = it->second->getSource();
    _userList.erase(clientFd);
    _operList.erase(clientFd);

    if (_userList.empty()) return 0;

    if (_operList.empty()) {
       pair<int, User *> nextOper;

       nextOper = *_userList.begin();
       _operList.insert(nextOper.first);
       broadcast(Message() << ":" << clientSource << "MODE" << getName() << "+o" << nextOper.second->getNickname());
    }
    return _userList.size();
}

User* Channel::findUser(const int clientFd) {
    map<int, User *>::iterator it;

    it = _userList.find(clientFd);
    if (it == _userList.end()) return NULL;
    return it->second;
}

User* Channel::findUser(const string& nickname) {
    map<int, User *>::iterator it;

    for(it = _userList.begin(); it != _userList.end(); ++it) {
        User *user = it->second;

        if (user->getNickname() == nickname) return user;
    }
    return NULL;
}

bool Channel::isUserOper(int clientFd) const {
    set<int>::iterator it;

    return (_operList.find(clientFd) != _operList.end());
}

void Channel::broadcast(const Message& msg, int ignoreFd) const {
    map<int, User *>::const_iterator it;

    for(it = _userList.begin(); it != _userList.end(); ++it) {
        if (it->first == ignoreFd) continue;

        it->second->addToReplyBuffer(msg.createReplyForm());
    }
}

void Channel::executeBot(const string& msgContent) {
	vector<string> params = Message::split(msgContent, ' ');
	string command = params[0];
	
	for (string::size_type i=0; i<command.length(); i++) command[i] = toupper(command[i]);
	if (command == "!HELP") {
		broadcast(Message() << ":" << SERVER_HOSTNAME << "PRIVMSG" << getName() << ":" 
                            << "Bot commands: .addmenu .deletemenu .showmenu .pickmenu");
	} else if (command == "!ADDMENU") {
		_bot.addMenu(params);
	} else if (command == "!DELETEMENU") {
		_bot.deleteMenu(params);
	} else if (command == "!SHOWMENU") {
		broadcast(Message() << ":" << SERVER_HOSTNAME << "PRIVMSG" << getName() << ":" 
                            << _bot.showMenu());
	} else if (command == "!PICKMENU") {
		broadcast(Message() << ":" << SERVER_HOSTNAME << "PRIVMSG" << getName() << ":" 
                            << _bot.pickMenu());
	}
}
