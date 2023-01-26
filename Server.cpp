#include "Server.hpp"
#include <iostream>

Server::Server(int port, string password): _fd(-1), _kq(-1), _port(port), _password(password) {
	struct sockaddr_in serverAddr;

	if ((_fd = socket(PF_INET, SOCK_STREAM, 0)) == -1)
		shutDown("socket() error");
	
	memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(_port);
	fcntl(_fd, F_SETFL, O_NONBLOCK);
	updateEvents(_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);

	if (bind(_fd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1)
        shutDown("bind() error");

	if (listen(_fd, 5) == -1)
        shutDown("listen() error");
}

Server::~Server() { }

void Server::disconnectClient(int clientFd) {
	User *user = _allUser[clientFd];
	map<string, Channel *>::iterator it;

	cout << "client disconnected: " << clientFd << '\n';
	for (it = _allChannel.begin(); it != _allChannel.end(); ++it) {
		it->second->deleteUser(clientFd);
	}
    close(clientFd);
    _allUser.erase(clientFd);
	delete user;
}

void Server::initKqueue() {
    if ((_kq = kqueue()) == -1)
        shutDown("kqueue() error");
}

void Server::updateEvents(int socket, int16_t filter, uint16_t flags, uint32_t fflags, intptr_t data, void *udata) {
	struct kevent event;

	EV_SET(&event, socket, filter, flags, fflags, data, udata);
	_eventCheckList.push_back(event);
}

User* Server::findClientByNickname(const string& nickname) {
	map<int, User*>::const_iterator it;
	for (it = _allUser.cbegin(); it != _allUser.end(); ++it) {
		if (it->second->getNickname() == nickname) return it->second;
	}
	return NULL;
}

Channel* Server::findChannelByName(const string& name) {
	map<string, Channel *>::const_iterator it;
	for (it = _allChannel.cbegin(); it != _allChannel.end(); ++it) {
		if (it->second->getName() == name) return it->second;
	}
	return NULL;
}

Channel* Server::addChannel(const string& name) {
	Channel *ch;

	ch = new Channel(name);
	cout << "channel added: " << name << '\n';
	_allChannel.insert(pair<string, Channel *>(name, ch));
	return ch;
}

void Server::deleteChannel(const string& name) {
	cout << "channel deleted: " << name << '\n';
	_allChannel.erase(name);
}

void Server::acceptNewClient(void) {
	int clientSocket;
	User *user;

	if ((clientSocket = accept(_fd, NULL, NULL)) == -1)
		shutDown("accept() error");
	cout << "accept new client: " << clientSocket << endl;
	fcntl(clientSocket, F_SETFL, O_NONBLOCK);

	/* add event for client socket - add read && write event */
	updateEvents(clientSocket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
	updateEvents(clientSocket, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
	user = new User(clientSocket);
	_allUser.insert(pair<int, User *>(clientSocket, user));
}

void Server::readDataFromClient(const struct kevent& event) {
	char buf[513];
	User* targetUser = _allUser[event.ident];
	int readBytes;

	readBytes = read(event.ident, buf, 512);
	if (readBytes <= 0) {
		cerr << "client read error!" << endl;
		disconnectClient(event.ident);
	} else {
		buf[readBytes] = '\0';
		targetUser->addToCmdBuffer(buf);
		handleMessageFromBuffer(targetUser);
	}
}

void Server::sendDataToClient(const struct kevent& event) {
	User *targetUser = _allUser[event.ident];
	int readBytes;

	if (targetUser->getReplyBuffer().empty()) return;

	if ((readBytes = write(event.ident, targetUser->getReplyBuffer().c_str(), targetUser->getReplyBuffer().length()) == -1))
	{
		cerr << "client write error!" << endl;
		disconnectClient(event.ident);  
	} else {
		targetUser->setReplyBuffer("");
	}
}

void Server::handleEvent(const struct kevent& event) {
	if (event.flags & EV_ERROR)
	{
		if (event.ident == (const uintptr_t)_fd)
			shutDown("server socket error");
		else
		{
			cerr << "client socket error" << endl;
			disconnectClient(event.ident);
		}
	}
	else if (event.filter == EVFILT_READ)
	{
		if (event.ident == (const uintptr_t)_fd)
			acceptNewClient();
		else
			readDataFromClient(event);
	}
	else if (event.filter == EVFILT_WRITE)
		sendDataToClient(event);
}

void Server::handleMessageFromBuffer(User* user) {
	size_t crlfPos;

	while ((crlfPos = checkCmdBuffer(user)) != string::npos) {
		if (crlfPos == 0) {
			user->setCmdBuffer(user->getCmdBuffer().substr(1, string::npos));
			continue;
		}
		Message msg(user->getCmdBuffer().substr(0, crlfPos));
		user->setCmdBuffer(user->getCmdBuffer().substr(crlfPos + 1, string::npos));
		runCommand(user, msg);
	}
}

size_t Server::checkCmdBuffer(const User *user) {
	const size_t	crPos = user->getCmdBuffer().find(CR, 0);
	const size_t	lfPos = user->getCmdBuffer().find(LF, 0);

	if (crPos == string::npos && lfPos == string::npos) return string::npos;
	if (lfPos == string::npos) return crPos;
	if (crPos == string::npos) return lfPos;
	return min(crPos, lfPos);
}

void Server::runCommand(User* user, Message& msg) {
    if (msg.getCommand() == "PRIVMSG") cmdPrivmsg(user, msg);
    else if (msg.getCommand() == "JOIN") cmdJoin(user, msg);
    else if (msg.getCommand() == "PART") cmdPart(user, msg);
}


void Server::cmdPrivmsg(User* user, Message& msg) {
    if (msg.getParams().size() != 2) return ;

    vector<string> targetList = msg.split(msg.getParams()[0], ',');
    for (vector<string>::const_iterator it = targetList.begin(); it != targetList.end(); ++it) {
        string targetName = *it;
        if (targetName[0] == '#') {
            Channel *targetChannel;

            targetChannel = findChannelByName(targetName.substr(1, string::npos));
            if (targetChannel == NULL) continue;
            targetChannel->broadcast(msg.getParams()[1] + '\n', user->getFd());
        } else {
            User *targetUser;

            targetUser = findClientByNickname(targetName);
            if (targetUser == NULL) continue;
            targetUser->addToReplyBuffer(msg.getParams()[1] + '\n'); // Format.. 
        }
    }
}

void Server::cmdJoin(User* user, Message& msg) {
    if (msg.getParams().size() == 0 || msg.getParams().size() > 2) return ;
    
    vector<string> targetList = msg.split(msg.getParams()[0], ',');
    if (targetList.size() == 1 && targetList[0] == "0") {
        vector<string> removeWaitingChannels;
        cout << user->getNickname() << " LEAVE FROM ALL CHANNELS" << endl;
        for (map<string, Channel *>::iterator it = _allChannel.begin(); it != _allChannel.end(); ++it) {
            const int remainUserOfChannel = it->second->deleteUser(user->getFd());
            if (remainUserOfChannel == 0) removeWaitingChannels.push_back(it->second->getName());
        }
        for (vector<string>::iterator it = removeWaitingChannels.begin(); it != removeWaitingChannels.end(); ++it) {
            deleteChannel(*it);
        }
        return ;
    }

    for (vector<string>::const_iterator it = targetList.begin(); it != targetList.end(); ++it) {
        string targetChannelName = *it;
        if (targetChannelName[0] != '#') continue;

        Channel *targetChannel;

        targetChannel = findChannelByName(targetChannelName.substr(1, string::npos));
        if (targetChannel == NULL) {
            targetChannel = addChannel(targetChannelName.substr(1, string::npos));
        }
        targetChannel->addUser(user->getFd(), user);
        // channel에 유저 들어옴 알림 -> PRIVATE .. format... :#CHANNEL PRIVMSG #CHANNEL :message
    }
}

void Server::cmdPart(User* user, Message& msg) {
    if (msg.getParams().size() != 1 && msg.getParams().size() != 2) return ;

    string partNotiMessage = user->getNickname();
    if (msg.getParams().size() == 2) partNotiMessage.append(msg.getParams()[1]);
    else partNotiMessage = partNotiMessage.append(DEFAULT_PART_MESSAGE);

    vector<string> targetList = msg.split(msg.getParams()[0], ',');
    for (vector<string>::const_iterator it = targetList.begin(); it != targetList.end(); ++it) {
        string targetChannelName = *it;

        if (targetChannelName[0] != '#') continue;

        Channel *targetChannel;
        targetChannel = findChannelByName(targetChannelName.substr(1, string::npos));
        if (targetChannel == NULL) continue;

        const int remainUserOfChannel = targetChannel->deleteUser(user->getFd());
        if (remainUserOfChannel == 0) deleteChannel(targetChannelName.substr(1, string::npos));
        else targetChannel->broadcast(partNotiMessage + '\n');
    }
}

void Server::cmdPass(User *user, Message& msg) {
	if (msg.getParams().size() != 1) return ;

	// auth -> password ERR {

	// }
	user->setPassword(msg.getParams()[0]);
}

void Server::cmdNick(User *user, Message& msg) {
	if (msg.getParams().size() != 1) {
		// msg.createForm(source, replyCmd, target, msg)
		// reply: ERR_NORECIPIENT(411) "<source> <client> :No recipient given (<command>)"
		return ;
	}
	const string requestNickname = msg.getParams()[0];

	if (requestNickname.length() == 0) {
		// reply: ERR_NONICKNAMEGIVEN(431)
		return ;
	}
	if (findClientByNickname(requestNickname) != NULL) {
		// reply: ERR_NICKNAMEINUSE(433) "<source> <client> <nick> :Nickname is already in use"
		return ;
	}
	
	user->setNickname(requestNickname);
	if (!user->getAuth() && !user->getUsername().empty()) {
		if (user->getPassword() == _password) user->setAuth();
		else disconnectClient(user->getFd());
	}
	//reply: "<source> NICK <nickname>"
}

void Server::cmdUser(User *user, Message& msg) {
	if (msg.getParams().size() != 4) return ; // <username> <hostname> <servername> <realname>
	if (user->getUsername().length() != 0) { // 이미 set되어 있는 user
		// reply : ERR_ALREADYREGISTRED(462)
		return ;
	}
	
	const string requestUsername = msg.getParams()[0]; //: username
	
	if (requestUsername.length() == 0) {
		// reply: ERR_NEEDMOREPARAMS(461)
	}
	
	user->setUsername(requestUsername);
	if (!user->getNickname().empty()) {
		if (user->getPassword() == _password) user->setAuth();
		else disconnectClient(user->getFd());
	}
	// reply
}

void Server::run() {
	int numOfEvents;
	
	initKqueue();
	while (1) {
        numOfEvents = kevent(_kq, &_eventCheckList[0], _eventCheckList.size(), _waitingEvents, 8, NULL);
        if (numOfEvents == -1)
            shutDown("kevent() error");
	
        _eventCheckList.clear();
        for (int i = 0; i < numOfEvents; ++i)
            handleEvent(_waitingEvents[i]);
    }
}

void Server::shutDown(const string& msg) {
	if (_fd != -1)
		close(_fd);
	if (_kq != -1)
		close(_kq);
	for (map<int, User *>::iterator it = _allUser.begin(); it != _allUser.end(); it++) {
		close(it->first);
		delete it->second;
	}
	for (map<string, Channel *>::iterator it = _allChannel.begin(); it != _allChannel.end(); it++) {
		delete it->second;
	}
	cerr << msg << endl;
	exit(EXIT_FAILURE);
}