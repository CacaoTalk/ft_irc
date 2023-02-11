#include "Server.hpp"
#include <iostream>

/**
 * @brief Construct a new Server:: Create a socket and wait for the client to connect.
 * 
 * @param port The port number that the client will use to connect to 
 * 	the IRC server from which it was created.
 * 	It will be get by argv[1].
 * @param password Password to check when connecting to the server.
 * 	Compare to the value delivered by the client using the PASS command.
 * 	It will be get by argv[2].
 */
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

/**
 * @brief Destroy the Server:: Server object
 */
Server::~Server() { }

/**
 * @brief Create kququq.
 * @throw Throw runtime_error if kqueue creation fails.
 */
void Server::initKqueue(void) {
    if ((_kq = kqueue()) == ERR_RETURN)
        throw(runtime_error("kqueue() error"));
}

/**
 * @brief Processes the registration of sockets and events to be managed by kqueue.
 * 
 * @param socket Client socket fd
 * @param filter Event filter. EVFILT_REAT, EVFILT_WRITE
 * @param flags Handle event. EV_ADD, EV_ENABLE
 * @param fflags Flag depend on filter. Not used.
 * @param data Data value depend on filter. Not used.
 * @param udata User-data that can be used at the event return. Not used.
 */
void Server::updateEvents(int socket, int16_t filter, uint16_t flags, uint32_t fflags, intptr_t data, void *udata) {
	struct kevent event;

	EV_SET(&event, socket, filter, flags, fflags, data, udata);
	_eventCheckList.push_back(event);
}

/**
 * @brief If a new client connects, assign a new socket to connect.
 * 	If possible, create a user instance and connect it to the socket you created.
 * 
 * @throw new or container.insert can throw exception.
 */
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
	if (_allUser.size() >= MAX_USER_NUM) {
		cout << "Server reached max number of user" << endl;
		close(clientSocket);
		return ;
	}
	fcntl(clientSocket, F_SETFL, O_NONBLOCK);
	inet_ntop(AF_INET, &clientAddr.sin_addr, hostStr, INET_ADDRSTRLEN);
	cout << "accept new client: " << clientSocket << " / Host : " << hostStr << endl;
	fcntl(clientSocket, F_SETFL, O_NONBLOCK);

	updateEvents(clientSocket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
	updateEvents(clientSocket, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);

	user = new User(clientSocket, hostStr);
	_allUser.insert(make_pair(clientSocket, user));
}

/**
 * @brief Read from the client socket and save it to the cmd buffer for that user.
 * 	It then calls a function that checks the cmd buffer for that user.
 * 	This function will be called when a read event occurs on that client.
 * 
 * @param event Event information delivered by kqueue.
 */
void Server::recvDataFromClient(const struct kevent& event) {
	char buf[513];
	map<int, User *>::iterator it = _allUser.find(event.ident);
	User* targetUser = it->second;
	int recvBytes;

	if (it == _allUser.end()) return ;

	recvBytes = recv(event.ident, buf, 512, 0);
	if (recvBytes <= 0) {
		if (recvBytes == ERR_RETURN && errno == EAGAIN) {
			errno = 0;
			return;
		}
		cerr << "client recv error!" << endl;
		targetUser->broadcastToMyChannels(Message() << ":" << targetUser->getSource() << "QUIT" << ":" << "Client closed connection", event.ident);
		disconnectClient(event.ident);
	} else {
		buf[recvBytes] = '\0';
		targetUser->addToCmdBuffer(buf);
		handleMessageFromBuffer(targetUser);
	}
}

/**
 * @brief Pass the send buffer contents that the user has to the client.
 * 	This function will be called when a write event occurs on that client.
 * 
 * @param event Event information delivered by kqueue.
 */
void Server::sendDataToClient(const struct kevent& event) {
	map<int, User *>::iterator it = _allUser.find(event.ident);
	User* targetUser = it->second;
	int sendBytes;

	if (it == _allUser.end()) return ;
	if (targetUser->getReplyBuffer().empty()) return;

	sendBytes = send(event.ident, targetUser->getReplyBuffer().c_str(), targetUser->getReplyBuffer().length(), 0);
	if (sendBytes == ERR_RETURN) {
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			errno = 0;
			return ;
		}
		cerr << "client send error!" << endl;
		targetUser->broadcastToMyChannels(Message() << ":" << targetUser->getSource() << "QUIT" << ":" << "Client closed connection", event.ident);
		disconnectClient(event.ident);  
	} else {

		targetUser->setReplyBuffer(targetUser->getReplyBuffer().substr(sendBytes));
		if (targetUser->getIsQuiting() && targetUser->getReplyBuffer().empty()) disconnectClient(event.ident);
	}
}

/**
 * @brief Manage events from fd managed by kqueue.
 *	Socket error handling, new client connections, and read/write processing from existing clients.
 * 
 * @param event Event information delivered by kqueue.
 */
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
			recvDataFromClient(event);
	} else if (event.filter == EVFILT_WRITE)
		sendDataToClient(event);
}

/**
 * @brief Passes messages truncated to CR/LF characters to the command processing function.
 * Remove the passed string from the user's cmd buffer.
 * 
 * @param user User to check buffer
 */
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

/**
 * @brief Verify that the user's cmd buffer has characters (CR or LF).
 * If any, returns the position of the first CR/LF characters.
 * 
 * @param user User to check buffer
 * @return size_t : Position of CR/LF character. If it does not exist, return string::npos.
 */
size_t Server::checkCmdBuffer(const User *user) const {
	const size_t	crPos = user->getCmdBuffer().find(CR, 0);
	const size_t	lfPos = user->getCmdBuffer().find(LF, 0);

	if (crPos == string::npos && lfPos == string::npos) return string::npos;
	if (lfPos == string::npos) return crPos;
	if (crPos == string::npos) return lfPos;
	return min(crPos, lfPos);
}

/**
 * @brief Gets the entire channel managed by the server.
 * 
 * @return const map<string, Channel *>& : Returns a map whose key is the channel name and 
 * 	value is the channel instance pointer.
 */
const map<string, Channel *>& Server::getAllChannel(void) const {
	return _allChannel;
}

/**
 * @brief Search by user's nickname.
 * 
 * @param nickname Nickname for find
 * @return User* : Returns the pointer to the user instance of the user found.
 * 	Returns NULL if not found.
 */
User* Server::findClientByNickname(const string& nickname) const {
	map<int, User*>::const_iterator it;
	for (it = _allUser.cbegin(); it != _allUser.end(); ++it) {
		if (it->second->getNickname() == nickname) return it->second;
	}
	return NULL;
}

/**
 * @brief Search by channel name.
 * 
 * @param name Channel name for find
 * @return Channel* : Returns the pointer to the channel instance of the channel found.
 * 	Returns NULL if not found.
 */
Channel* Server::findChannelByName(const string& name) const {
	if (name[0] != '#') return NULL;
	
	map<string, Channel *>::const_iterator it;
	for (it = _allChannel.cbegin(); it != _allChannel.end(); ++it) {
		if (it->second->getName() == name) return it->second;
	}
	return NULL;
}

/**
 * @brief Verify that it matches the server password.
 * 
 * @param password Password passed by user with PASS command
 * @return true : Password match / if not return
 * @return false 
 */
bool Server::checkPassword(const string& password) const {
	if (_password == password) return true;

	return false;
}

/**
 * @brief Add a new channel to the server.
 * 
 * @param name Channel name to add
 * @return Channel* : Returns the pointer of the added channel instance.
 * 	Returns NULL if it fails.
 * @throw container.insert can throw exception
 */
Channel* Server::addChannel(const string& name) {
	if (_allChannel.size() >= MAX_CHANNEL_NUM) return NULL;
	
	Channel *ch;

	ch = new Channel(name);
	_allChannel.insert(make_pair(name, ch));
	cout << "channel added: " << name << '\n';
	return ch;
}

/**
 * @brief Deletes a channel that exists on the server.
 * 
 * @param name Channel name to delete
 */
void Server::deleteChannel(const string& name) {
	map<string, Channel *>::iterator it = _allChannel.find(name);
	Channel *ch = it->second;

	if (it == _allChannel.end()) return ;
	
	cout << "Delete channel from server: " << name << '\n';
	_allChannel.erase(name);
	delete ch;
}

/**
 * @brief Disconnects a specific client from a server.
 * Delete user, withdraw from the channel to which 
 * 	the user belonged (delete channel if necessary)
 * 
 * @param clientFd Socket fd of client to disconnect
 */
void Server::disconnectClient(int clientFd) {
	map<int, User *>::iterator it = _allUser.find(clientFd);
	User* targetUser = it->second;

	if (it == _allUser.end()) return ;

    _allUser.erase(clientFd);
	const vector<Channel *> userChannelList = targetUser->getMyAllChannel();
	for (vector<Channel *>::const_iterator it = userChannelList.begin(); it != userChannelList.end(); ++it) {
		const int remainUsers = (*it)->deleteUser(targetUser->getFd());
		if (remainUsers == 0) deleteChannel((*it)->getName());
	}
	targetUser->clearMyChannelList();
	delete targetUser;
	cout << "client disconnected: " << clientFd << '\n';
}

/**
 * @brief Manage clients that connect to the server's sockets.
 * 	This function turns an infinite loop.
 */
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

/**
 * @brief Called when the server shuts down abnormally.
 * 
 * @param msg Error message to output to console
 */
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
