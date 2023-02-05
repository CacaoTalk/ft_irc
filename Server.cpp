#include "Server.hpp"
#include <iostream>

Server::Server(int port, string password): _fd(UNDEFINED_FD), _kq(UNDEFINED_FD), _port(port), _password(password), _command(*this) {
	struct sockaddr_in serverAddr;

	if ((_fd = socket(PF_INET, SOCK_STREAM, 0)) == ERR_RETURN)
		shutDown("socket() error");
	
	memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(_port);
	fcntl(_fd, F_SETFL, O_NONBLOCK);
	updateEvents(_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);

	if (::bind(_fd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == ERR_RETURN)
        shutDown("bind() error");

	if (listen(_fd, 5) == ERR_RETURN)
        shutDown("listen() error");
}

Server::~Server() { }

void Server::initKqueue(void) {
    if ((_kq = kqueue()) == ERR_RETURN)
        throw(runtime_error("kqueue() error"));
}

void Server::updateEvents(int socket, int16_t filter, uint16_t flags, uint32_t fflags, intptr_t data, void *udata) {
	struct kevent event;

	EV_SET(&event, socket, filter, flags, fflags, data, udata);
	_eventCheckList.push_back(event);
}

void Server::acceptNewClient(void) {
	int clientSocket;
	struct sockaddr_in clientAddr;
	socklen_t addrLen = sizeof(clientAddr);
	char hostStr[INET_ADDRSTRLEN];
	User *user;

	memset(&clientAddr, 0, sizeof(clientAddr));
	memset(hostStr, 0, sizeof(hostStr));
	if ((clientSocket = accept(_fd, (struct sockaddr *)&clientAddr, &addrLen)) == ERR_RETURN)
		throw(runtime_error("accept() error"));
	fcntl(clientSocket, F_SETFL, O_NONBLOCK);
	inet_ntop(AF_INET, &clientAddr.sin_addr, hostStr, INET_ADDRSTRLEN);
	cout << "accept new client: " << clientSocket << " / Host : " << hostStr << endl;
	fcntl(clientSocket, F_SETFL, O_NONBLOCK);

	updateEvents(clientSocket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
	updateEvents(clientSocket, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);

	user = new User(clientSocket, hostStr);
	_allUser.insert(make_pair(clientSocket, user));
}

void Server::readDataFromClient(const struct kevent& event) {
	char buf[513];
	map<int, User *>::iterator it = _allUser.find(event.ident);
	User* targetUser = it->second;
	int readBytes;

	if (it == _allUser.end()) return ;

	readBytes = read(event.ident, buf, 512);
	if (readBytes <= 0) {
		if (readBytes == ERR_RETURN && errno == EAGAIN) {
			errno = 0;
			return;
		}
		cerr << "client read error!" << endl;
		targetUser->broadcastToMyChannels(Message() << ":" << targetUser->getSource() << "QUIT" << ":" << "Client closed connection", event.ident);
		disconnectClient(event.ident);
	} else {
		buf[readBytes] = '\0';
		targetUser->addToCmdBuffer(buf);
		handleMessageFromBuffer(targetUser);
	}
}

void Server::sendDataToClient(const struct kevent& event) {
	map<int, User *>::iterator it = _allUser.find(event.ident);
	User* targetUser = it->second;
	int readBytes;

	if (it == _allUser.end()) return ;
	if (targetUser->getReplyBuffer().empty()) return;

	readBytes = write(event.ident, targetUser->getReplyBuffer().c_str(), targetUser->getReplyBuffer().length());
	if (readBytes == ERR_RETURN) {
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			errno = 0;
			return ;
		}
		cerr << "client write error!" << endl;
		targetUser->broadcastToMyChannels(Message() << ":" << targetUser->getSource() << "QUIT" << ":" << "Client closed connection", event.ident);
		disconnectClient(event.ident);  
	} else {
		targetUser->setReplyBuffer(targetUser->getReplyBuffer().substr(readBytes));
	}
}

void Server::handleEvent(const struct kevent& event) {
	if (event.flags & EV_ERROR) {
		if (event.ident == (const uintptr_t)_fd)
			throw(runtime_error("server socket error"));
		else {
			User *targetUser = _allUser[event.ident];

			cerr << "client socket error" << endl;
			targetUser->broadcastToMyChannels(Message() << ":" << targetUser->getSource() << "QUIT" << ":" << "Client closed connection", event.ident);
			disconnectClient(event.ident);
		}
	} else if (event.filter == EVFILT_READ) {
		if (event.ident == (const uintptr_t)_fd)
			acceptNewClient();
		else
			readDataFromClient(event);
	} else if (event.filter == EVFILT_WRITE)
		sendDataToClient(event);
}

void Server::handleMessageFromBuffer(User* user) {
	size_t crlfPos;

	while ((crlfPos = checkCmdBuffer(user)) != string::npos) {
		if (crlfPos == 0) {
			user->setCmdBuffer(user->getCmdBuffer().substr(1));
			continue;
		}
		Message msg(user->getCmdBuffer().substr(0, crlfPos));
		user->setCmdBuffer(user->getCmdBuffer().substr(crlfPos + 1));
		if (!_command.run(user, msg)) break;
	}
}

size_t Server::checkCmdBuffer(const User *user) const {
	const size_t	crPos = user->getCmdBuffer().find(CR, 0);
	const size_t	lfPos = user->getCmdBuffer().find(LF, 0);

	if (crPos == string::npos && lfPos == string::npos) return string::npos;
	if (lfPos == string::npos) return crPos;
	if (crPos == string::npos) return lfPos;
	return min(crPos, lfPos);
}

const map<string, Channel *>& Server::getAllChannel(void) const {
	return _allChannel;
}

User* Server::findClientByNickname(const string& nickname) const {
	map<int, User*>::const_iterator it;
	for (it = _allUser.cbegin(); it != _allUser.end(); ++it) {
		if (it->second->getNickname() == nickname) return it->second;
	}
	return NULL;
}

Channel* Server::findChannelByName(const string& name) const {
	if (name[0] != '#') return NULL;
	
	map<string, Channel *>::const_iterator it;
	for (it = _allChannel.cbegin(); it != _allChannel.end(); ++it) {
		if (it->second->getName() == name) return it->second;
	}
	return NULL;
}

bool Server::checkPassword(const string& password) const {
	if (_password == password) return true;

	return false;
}

Channel* Server::addChannel(const string& name) {
	Channel *ch;

	ch = new Channel(name);
	_allChannel.insert(make_pair(name, ch));
	cout << "channel added: " << name << '\n';
	return ch;
}

void Server::deleteChannel(const string& name) {
	map<string, Channel *>::iterator it = _allChannel.find(name);
	Channel *ch = it->second;

	if (it == _allChannel.end()) return ;
	
	_allChannel.erase(name);
	delete ch;
	cout << "channel deleted: " << name << '\n';
}

void Server::disconnectClient(int clientFd) {
	map<int, User *>::iterator it = _allUser.find(clientFd);
	User* targetUser = it->second;

	if (it == _allUser.end()) return ;
    _allUser.erase(clientFd);
	delete targetUser;
	cout << "client disconnected: " << clientFd << '\n';
}

void Server::run() {
	int numOfEvents;
	
	initKqueue();
	cout << "listening..." << endl;
	while (1) {
        numOfEvents = kevent(_kq, &_eventCheckList[0], _eventCheckList.size(), _waitingEvents, 8, NULL);
        if (numOfEvents == ERR_RETURN)
            shutDown("kevent() error");
	
        _eventCheckList.clear();
        for (int i = 0; i < numOfEvents; ++i)
            handleEvent(_waitingEvents[i]);
    }
}

void Server::shutDown(const string& msg) {
	if (_fd != UNDEFINED_FD)
		close(_fd);
	if (_kq != UNDEFINED_FD)
		close(_kq);
	for (map<int, User *>::iterator it = _allUser.begin(); it != _allUser.end(); it++) {
		delete it->second;
	}
	for (map<string, Channel *>::iterator it = _allChannel.begin(); it != _allChannel.end(); it++) {
		delete it->second;
	}
	cerr << msg << endl;
	exit(EXIT_FAILURE);
}
