#include "User.hpp"
#include "Channel.hpp"
#include "Message.hpp"

/**
 * @brief Construct a new Channel:: Channel object
 * 
 * @param name name of channel
 */
Channel::Channel(const string& name): _name(name) {}

/**
 * @brief Destroy the Channel:: Channel object
 */
Channel::~Channel() { }

/**
 * @brief Getter
 * 
 * @return const string& : "<Channel name>"
 */
const string& Channel::getName(void) const {
    return _name;
}

/**
 * @brief Get users list of channel
 * 
 * @return const vector<string> : Users nickname list to vector. Channel operator has '\@' by prefix
 */
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


/**
 * @brief Add user to channel. If target user is the first of this channel, set to channel operator.
 * 
 * @param clientFd Socket fd of user
 * @param user User class pointer of user
 * @throw container.insert method can throw exception
 */
void Channel::addUser(int clientFd, User *user) {
    if (_userList.empty()) _operList.insert(clientFd);
    _userList.insert(make_pair(clientFd, user));
}

/**
 * @brief Delete user from channel. If target user was channel operator, set another user to channel operator.
 * 
 * @param clientFd Socket fd of user
 * @return int : Number of remain users after delete user. This is for delete channel if nobody in this channel.
 * @throw container.insert method can throw exception
 */
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

/**
 * @brief Find user in channel by socket fd
 * 
 * @param clientFd Socket fd of user
 * @return User* : User class pointer
 * @exception NULL : Target user not exist in this channel
 */
User* Channel::findUser(const int clientFd) {
    map<int, User *>::iterator it;

    it = _userList.find(clientFd);
    if (it == _userList.end()) return NULL;
    return it->second;
}

/**
 * @brief Find user in channel by nickname
 * 
 * @param nickname nickname of user
 * @return User* : User class pointer
 * @exception NULL : Target user not exist in this channel
 */
User* Channel::findUser(const string& nickname) {
    map<int, User *>::iterator it;

    for(it = _userList.begin(); it != _userList.end(); ++it) {
        User *user = it->second;

        if (user->getNickname() == nickname) return user;
    }
    return NULL;
}

/**
 * @brief Check user is channel operator.
 * 
 * @param clientFd Socket fd of user
 * @return true : User is channel operator
 * @return false : User is not channel operator OR not exist in this channel
 */
bool Channel::isUserOper(int clientFd) const {
    set<int>::iterator it;

    return (_operList.find(clientFd) != _operList.end());
}

/**
 * @brief Send message to all users in this channel
 * 
 * @param msg Message
 * @param ignoreFd Socket fd that not wnat to be sent. -1(default argument) means send to all user.
 */
void Channel::broadcast(const Message& msg, int ignoreFd) const {
    map<int, User *>::const_iterator it;

    for(it = _userList.begin(); it != _userList.end(); ++it) {
        if (it->first == ignoreFd) continue;

        it->second->addToReplyBuffer(msg.createReplyForm());
    }
}

/**
 * @brief Bot command middleware. Runs regardless of upper/lower case.
 * 
 * @param msgContent Message that sent by PRIVMSG command that content start with '!'
 */
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
