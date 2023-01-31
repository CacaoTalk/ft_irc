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
	if (name[0] != '#') return NULL;
	
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
	else if (msg.getCommand() == "PASS") cmdPass(user, msg);
	else if (msg.getCommand() == "NICK") cmdNick(user, msg);
	else if (msg.getCommand() == "USER") cmdUser(user, msg);
	else if (msg.getCommand() == "PING") cmdPing(user, msg);
	else if (msg.getCommand() == "QUIT") cmdQuit(user, msg);
	else if (msg.getCommand() == "KICK") cmdKick(user, msg);
	else if (msg.getCommand() == "NOTICE") cmdNotice(user, msg);
}


void Server::cmdPrivmsg(User* user, Message& msg) {
    if (msg.getParams().size() < 2) {
		Message replyMsg;
		replyMsg << ":" << SERVER_HOSTNAME << ERR_NORECIPIENT << user->getNickname() << ERR_NORECIPIENT_MSG << "(PRIVMSG)";
		user->addToReplyBuffer(replyMsg.createReplyForm());
		return ;
	}

    vector<string> targetList = msg.split(msg.getParams()[0], ',');
    for (vector<string>::const_iterator it = targetList.begin(); it != targetList.end(); ++it) {
        string targetName = *it;
        if (targetName[0] == '#') {
            Channel *targetChannel;

            targetChannel = findChannelByName(targetName);
            if (targetChannel == NULL) {
				Message replyMsg;
				replyMsg << ":" << SERVER_HOSTNAME << ERR_NOSUCHNICK << user->getNickname() << targetName << ERR_NOSUCHNICK_MSG;
				user->addToReplyBuffer(replyMsg.createReplyForm());
				continue;
			}
			Message replyMsg;
			replyMsg << ":" << user->getNickname() << msg.getCommand() << targetChannel->getName() << msg.getParams()[1];
            targetChannel->broadcast(replyMsg.createReplyForm(), user->getFd());
        } else {
            User *targetUser;

            targetUser = findClientByNickname(targetName);
            if (targetUser == NULL) {
				Message replyMsg;
				replyMsg << string(":").append(SERVER_HOSTNAME) << ERR_NOSUCHNICK << user->getNickname() << targetName << ERR_NOSUCHNICK_MSG;
				user->addToReplyBuffer(replyMsg.createReplyForm());
				continue;
			}
			Message replyMsg;
			replyMsg << ":" << user->getNickname() << msg.getCommand() << targetUser->getNickname() << msg.getParams()[1];
            targetUser->addToReplyBuffer(replyMsg.createReplyForm());
        }
    }
}

void Server::cmdJoin(User* user, Message& msg) {
    if (msg.getParams().size() == 0) {
		Message replyMsg;
		replyMsg << ":" << SERVER_HOSTNAME << ERR_NEEDMOREPARAMS << user->getNickname() << msg.getCommand() << ERR_NEEDMOREPARAMS_MSG;
		user->addToReplyBuffer(replyMsg.createReplyForm());
		return ;
	}
    
    vector<string> targetList = msg.split(msg.getParams()[0], ',');
    if (targetList.size() == 1 && targetList[0] == "0") {
        vector<string> removeWaitingChannels;
        cout << user->getNickname() << " LEAVE FROM ALL CHANNELS" << endl;
        for (map<string, Channel *>::iterator it = _allChannel.begin(); it != _allChannel.end(); ++it) {
			Channel *targetChannel = it->second;
			User *userInChannel = targetChannel->findUser(user->getFd());

			if (userInChannel == NULL) continue;
			
            const int remainUserOfChannel = targetChannel->deleteUser(user->getFd());
			Message replyMsg;
			replyMsg << user->getNickname() << "PART" << targetChannel->getName();
			user->addToReplyBuffer(replyMsg.createReplyForm());
			targetChannel->broadcast(replyMsg.createReplyForm());
            if (remainUserOfChannel == 0) removeWaitingChannels.push_back(targetChannel->getName());
        }
        for (vector<string>::iterator it = removeWaitingChannels.begin(); it != removeWaitingChannels.end(); ++it) {
            deleteChannel(*it);
        }
        return ;
    }

    for (vector<string>::const_iterator it = targetList.begin(); it != targetList.end(); ++it) {
        string targetChannelName = *it;
        if (targetChannelName[0] != '#') {
			Message replyMsg;
			replyMsg << ":" << SERVER_HOSTNAME << ERR_NOSUCHCHANNEL << user->getNickname() << targetChannelName << ERR_NOSUCHCHANNEL_MSG;
			user->addToReplyBuffer(replyMsg.createReplyForm());
			continue;
		}

        Channel *targetChannel;

        targetChannel = findChannelByName(targetChannelName);
        if (targetChannel == NULL) {
            targetChannel = addChannel(targetChannelName);
        } else if (targetChannel->findUser(user->getFd()) != NULL) continue;
		
        targetChannel->addUser(user->getFd(), user);
		Message replyMsg[3];
		replyMsg[0] << ":" << user->getNickname() << msg.getCommand() << ":" << targetChannelName;
		replyMsg[1] << ":" << SERVER_HOSTNAME << RPL_NAMREPLY << user->getNickname() << "=" << targetChannelName << ":";
		vector<string> targetChannelUserList = targetChannel->getUserList();
		for (vector<string>::iterator it = targetChannelUserList.begin(); it != targetChannelUserList.end(); ++it) {
			replyMsg[1] << *it;
		}
		replyMsg[2] << ":" << SERVER_HOSTNAME << RPL_ENDOFNAMES << user->getNickname() << targetChannelName << RPL_ENDOFNAMES_MSG;
		targetChannel->broadcast(replyMsg[0].createReplyForm());
		user->addToReplyBuffer(replyMsg[1].createReplyForm());		
		user->addToReplyBuffer(replyMsg[2].createReplyForm());
    }
}

void Server::cmdPart(User* user, Message& msg) {
	if (msg.getParams().size() < 1) {
		Message replyMsg;
		replyMsg << ":" << SERVER_HOSTNAME << ERR_NEEDMOREPARAMS << user->getNickname() << msg.getCommand() << ERR_NEEDMOREPARAMS_MSG;
		user->addToReplyBuffer(replyMsg.createReplyForm());
		return ;
	}

    string partNotiMessage;
    if (msg.getParams().size() >= 2) {
		partNotiMessage.append(":");
		partNotiMessage.append(msg.getParams()[1]);
	}

    vector<string> targetList = msg.split(msg.getParams()[0], ',');
    for (vector<string>::const_iterator it = targetList.begin(); it != targetList.end(); ++it) {
        string targetChannelName = *it;

        Channel *targetChannel;
        targetChannel = findChannelByName(targetChannelName);

        if (targetChannel == NULL) {
			Message replyMsg;
			replyMsg << ":" << SERVER_HOSTNAME << ERR_NOSUCHCHANNEL << user->getNickname() << targetChannelName << ERR_NOSUCHCHANNEL_MSG;
			user->addToReplyBuffer(replyMsg.createReplyForm());
			continue;
		}
		if (targetChannel->findUser(user->getFd()) == NULL) {
			Message replyMsg;
			replyMsg << ":" << SERVER_HOSTNAME << ERR_NOTONCHANNEL << user->getNickname() << targetChannelName << ERR_NOTONCHANNEL_MSG;
			user->addToReplyBuffer(replyMsg.createReplyForm());
			continue;
		}
        const int remainUserOfChannel = targetChannel->deleteUser(user->getFd());
		Message replyMsg;
		replyMsg << user->getNickname() << "PART" << targetChannelName;
		user->addToReplyBuffer(replyMsg.createReplyForm());
		targetChannel->broadcast(replyMsg.createReplyForm());
        if (remainUserOfChannel == 0) deleteChannel(targetChannelName);
    }
}

void Server::cmdPass(User *user, Message& msg) {
	Message replyMsg;
	
	if (msg.getParams().size() < 1) {
		replyMsg << ":" << SERVER_HOSTNAME << ERR_NEEDMOREPARAMS << user->getNickname() << msg.getCommand() << ERR_NEEDMOREPARAMS_MSG;
		user->addToReplyBuffer(replyMsg.createReplyForm());
		return ;
	}
	if (user->getAuth()) {
		replyMsg << ":" << SERVER_HOSTNAME << ERR_ALREADYREGISTERED << user->getNickname() << ERR_ALREADYREGISTERED_MSG;
		user->addToReplyBuffer(replyMsg.createReplyForm());
		return ;
	}
	user->setPassword(msg.getParams()[0]);
}

void Server::cmdNick(User *user, Message& msg) {
	Message replyMsg;
	
	if (msg.getParams().size() < 1) {
		replyMsg << ":" << SERVER_HOSTNAME << ERR_NEEDMOREPARAMS << user->getNickname() << msg.getCommand() << ERR_NEEDMOREPARAMS_MSG;
		user->addToReplyBuffer(replyMsg.createReplyForm());
		return ;
	}
	const string requestNickname = msg.getParams()[0];

	if (requestNickname.length() == 0) {
		replyMsg << ":" << SERVER_HOSTNAME << ERR_NONICKNAMEGIVEN << user->getNickname() << ERR_NONICKNAMEGIVEN_MSG;
		user->addToReplyBuffer(replyMsg.createReplyForm());
		return ;
	}

	if (findClientByNickname(requestNickname) != NULL) {
		replyMsg << ":" << SERVER_HOSTNAME << ERR_NICKNAMEINUSE << user->getNickname() << requestNickname << ERR_NICKNAMEINUSE_MSG;
		user->addToReplyBuffer(replyMsg.createReplyForm());
		return ;
	}
	
	user->setNickname(requestNickname);
	if (!user->getAuth() && !user->getUsername().empty()) {
		if (user->getPassword() == _password) {
			user->setAuth();
			replyMsg << ":" << SERVER_HOSTNAME << RPL_WELCOME << user->getNickname() << ":Welcome to the" << SERVER_HOSTNAME <<  "Network" << requestNickname;
			user->addToReplyBuffer(replyMsg.createReplyForm());
		}
		else {
			replyMsg << ":" << SERVER_HOSTNAME << ERR_PASSWDMISMATCH << user->getNickname() << ERR_PASSWDMISMATCH_MSG;
			user->addToReplyBuffer(replyMsg.createReplyForm());
			disconnectClient(user->getFd());
		}
		return ;
	}
	replyMsg << ":" << user->getNickname() << msg.getCommand() << requestNickname;
	user->addToReplyBuffer(replyMsg.createReplyForm());
}

void Server::cmdUser(User *user, Message& msg) {
	Message replyMsg;
	
	if (msg.getParams().size() < 4) {
		replyMsg << ":" << SERVER_HOSTNAME << ERR_NEEDMOREPARAMS << user->getNickname() << msg.getCommand() << ERR_NEEDMOREPARAMS_MSG;
		user->addToReplyBuffer(replyMsg.createReplyForm());
		return ;
	}
	
	if (user->getAuth()) { // 이미 auth되어 있는 user
		replyMsg << ":" << SERVER_HOSTNAME << ERR_ALREADYREGISTERED << user->getNickname() << ERR_ALREADYREGISTERED_MSG;
		user->addToReplyBuffer(replyMsg.createReplyForm());
		return ;
	}
	
	const string requestUserNickname = msg.getParams()[0]; //username
	
	if (requestUserNickname.length() == 0) {
		replyMsg << ":" << SERVER_HOSTNAME << ERR_NEEDMOREPARAMS << user->getNickname() << msg.getCommand() << ERR_NEEDMOREPARAMS_MSG;
		user->addToReplyBuffer(replyMsg.createReplyForm());
		return ;
	}
	
	user->setUsername(requestUserNickname);
	if (!user->getNickname().empty()) {
		if (user->getPassword() == _password) {
			user->setAuth();
			replyMsg << ":" << SERVER_HOSTNAME << RPL_WELCOME << user->getNickname() << ":Welcome to the" << SERVER_HOSTNAME <<  "Network" << requestUserNickname;
			user->addToReplyBuffer(replyMsg.createReplyForm());
		}
		else {
			replyMsg << ":" << SERVER_HOSTNAME << ERR_PASSWDMISMATCH << user->getNickname() << ERR_PASSWDMISMATCH_MSG;
			user->addToReplyBuffer(replyMsg.createReplyForm());
			disconnectClient(user->getFd());
		}
		return ;
	}
}

void Server::cmdPing(User *user, Message& msg) {
	Message replyMsg;

	if (msg.getParams().size() < 1) {
		replyMsg << ":" << SERVER_HOSTNAME << ERR_NEEDMOREPARAMS << user->getNickname() << msg.getCommand() << ERR_NEEDMOREPARAMS_MSG;
		user->addToReplyBuffer(replyMsg.createReplyForm());
		return ;
	}

	if (msg.getParams()[0].empty()) {
		replyMsg << ":" << SERVER_HOSTNAME << ERR_NOORIGIN << user->getNickname() << ERR_NOORIGIN_MSG;
		user->addToReplyBuffer(replyMsg.createReplyForm());
		return ;
	}
	
	replyMsg << ":" << SERVER_HOSTNAME << "PONG" << SERVER_HOSTNAME << msg.getParams()[0];
	user->addToReplyBuffer(replyMsg.createReplyForm());
}

void Server::cmdQuit(User *user, Message& msg) {
	if (msg.getParams().size() < 1) {
		Message replyMsg;

		replyMsg << ":" << SERVER_HOSTNAME << ERR_NEEDMOREPARAMS << user->getNickname() << msg.getCommand() << ERR_NEEDMOREPARAMS_MSG;
		user->addToReplyBuffer(replyMsg.createReplyForm());
		return ;
	}

	string reason = ":Quit:";
	if (msg.getParams().size() == 1) reason += msg.getParams()[0];
	else reason += "leaving";
	
	Message replyMsg;
	replyMsg << ":" << user->getNickname() << msg.getCommand() << reason;
	
	int clientFd = user->getFd();
	map<string, Channel *>::iterator it;
	for (it = _allChannel.begin(); it != _allChannel.end(); ++it) {
		if (it->second->findUser(clientFd) != NULL) {
			it->second->broadcast(replyMsg.createReplyForm(), clientFd);
		}
	}
	disconnectClient(clientFd);
}

// 
void Server::cmdKick(User *user, Message& msg) {
	if (msg.getParams().size() < 2) {
		Message replyMsg;
		
		replyMsg << ":" << SERVER_HOSTNAME << ERR_NEEDMOREPARAMS << user->getNickname() << msg.getCommand() << ERR_NEEDMOREPARAMS_MSG;
		user->addToReplyBuffer(replyMsg.createReplyForm());
	}

	string reason;
	if (msg.getParams().size() >= 3) {
		reason.append(":");
		reason.append(msg.getParams()[2]);
	}
	
	// 해당 channel이 존재하는 지 check
	Channel *targetChannel = findChannelByName(msg.getParams()[0]);
	if (targetChannel == NULL) {
		Message replyMsg;

		replyMsg << ":" << SERVER_HOSTNAME << ERR_NOSUCHCHANNEL << user->getNickname() << msg.getParams()[0] << ERR_NOSUCHCHANNEL_MSG;
		user->addToReplyBuffer(replyMsg.createReplyForm());
		return ;
	}
	
	// User가 channel에 있는 지 check
	if (targetChannel->findUser(user->getFd()) == NULL) {\
		Message replyMsg;

		replyMsg << ":" << SERVER_HOSTNAME << ERR_NOTONCHANNEL << user->getNickname() << msg.getParams()[0] << ERR_NOTONCHANNEL_MSG;
		user->addToReplyBuffer(replyMsg.createReplyForm());
		return ;
	}

	// User가 해당 channel의 operator인지
	if (targetChannel->isUserOper(user->getFd()) == false) {
		Message replyMsg;

		replyMsg << ":" << SERVER_HOSTNAME << ERR_CHANOPRIVSNEEDED << user->getNickname() << msg.getParams()[0] << ERR_CHANOPRIVSNEEDED_MSG;
		user->addToReplyBuffer(replyMsg.createReplyForm());
		return ;
	}

	// iteration
	vector<string> targetUsers = msg.split(msg.getParams()[1], ',');
	for (vector<string>::const_iterator it = targetUsers.begin(); it != targetUsers.end(); ++it) {
		// target User가 channel에 존재하는지
		int targetFd = findClientByNickname(*it)->getFd();
		if (targetChannel->findUser(targetFd) == NULL) {
			Message replyMsg;
			replyMsg << ":" << SERVER_HOSTNAME << ERR_USERNOTINCHANNEL << user->getNickname() << *it << msg.getParams()[0] << ERR_USERNOTINCHANNEL_MSG;
			user->addToReplyBuffer(replyMsg.createReplyForm());
		}
		// 존재하면 Kick (그 channel에 deleteUser)
		Message replyMsg;
		
		replyMsg << ":" << user->getNickname() << msg.getCommand() << msg.getParams()[0] << *it << ":" << user->getNickname();
		targetChannel->broadcast(replyMsg.createReplyForm());
		const int remainUsers = targetChannel->deleteUser(targetFd);
		if (remainUsers == 0) deleteChannel(targetChannel->getName());
	}
}

void Server::cmdNotice(User *user, Message& msg) {
	Message replyMsg;
	
	if (msg.getParams().size() == 0) {
		replyMsg << ":" << SERVER_HOSTNAME << ERR_NORECIPIENT << user->getNickname() << ERR_NORECIPIENT_MSG << "(NOTICE)";
		user->addToReplyBuffer(replyMsg.createReplyForm());
		return;
	}
	if (msg.getParams().size() == 1) {
		replyMsg << ":" << SERVER_HOSTNAME << ERR_NOTEXTTOSEND << user->getNickname() << msg.getCommand() << ERR_NOTEXTTOSEND_MSG;
		user->addToReplyBuffer(replyMsg.createReplyForm());
		return;
	}

    vector<string> targetList = msg.split(msg.getParams()[0], ',');
    for (vector<string>::const_iterator it = targetList.begin(); it != targetList.end(); ++it) {
        string targetName = *it;
        if (targetName[0] == '#') {
            Channel *targetChannel;

            targetChannel = findChannelByName(targetName);
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