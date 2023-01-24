#include "Server.hpp"
#include <iostream>

Server::Server(): _fd(-1), _kq(-1) {
	struct sockaddr_in serverAddr;

	if ((_fd = socket(PF_INET, SOCK_STREAM, 0)) == -1)
		shutDown("socket() error");
	
	memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(6667);
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

User* Server::findClientByNickname(string nickname) {
	map<int, User*>::const_iterator it;
	for (it = _allUser.cbegin(); it != _allUser.end(); ++it) {
		if (it->second->getNickname() == nickname) return it->second;
	}
	return NULL;
}

void Server::addChannel(const string& name) {
	Channel *ch;

	ch = new Channel(name);
	cout << "channel added: " << name << '\n';
	_allChannel.insert(pair<string, Channel *>(name, ch));
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
	// channel에 추가 (추후 join 시에만 추가하도록 수정)
	_allChannel.begin()->second->addUser(clientSocket, user);
}

void Server::readDataFromClient(const struct kevent& event) {
	char buf[512];
	// User* targetUser = _allUser[event.ident];
	int readBytes;

	readBytes = read(event.ident, buf, sizeof(buf));
	if (readBytes <= 0) {
		cerr << "client read error!" << endl;
		disconnectClient(event.ident);
	} else {
		buf[readBytes] = '\0';
		// targetUser->addToCmdBuffer(buf);
		// CR LF check -> exist -> parsing
		// parsed msg -> command에 맞게 처리
		_allChannel.begin()->second->broadcast(buf, event.ident);
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

void Server::run() {
	int numOfEvents;

	// temp Channel
	addChannel("tempChannel");
	
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