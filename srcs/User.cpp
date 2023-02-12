#include <unistd.h>
#include "User.hpp"
#include "Channel.hpp"
#include "Message.hpp"

/**
 * @brief Construct a new User:: User object
 * 
 * @param fd Client socket fd
 * @param host Client host address(ipv4)
 */
User::User(int fd, const string& host) : _fd(fd), _host(host), _auth(false), _isQuiting(false) { }

/**
 * @brief Destroy the User:: Close client socket fd
 */
User::~User() {
    close(_fd);
}

/**
 * @brief Get client socket fd
 * 
 * @return int : client socket fd
 */
int User::getFd(void) const {
    return _fd;
}

/**
 * @brief Get client host address.
 * 
 * @return const string& : Host address(ipv4)
 */
const string& User::getHost(void) const {
    return _host;
}

/**
 * @brief Get the password required to connect to the server.
 * This value is set to the parameter of the PASS command sent by that user.
 * 
 * @return const string& : Password sent by that user.
 */
const string& User::getPassword(void) const {
    return _password;
}

/**
 * @brief Get user nickname.
 * 
 * @return const string : Nickname of user.
 *  Returns "*" if it is before the client set on a nickname. This can only happen before authentication.
 */
const string User::getNickname(void) const {
    if (_nickname.empty()) return "*";
    
    return _nickname;
}

/**
 * @brief Get user source.
 * 
 * @return const string : "<nickname>@<host_addr>"
 */
const string User::getSource(void) const {
    string source = getNickname();
    if (source.empty()) source = "*";
    source = source + "@" + _host;

    return source;
}

/**
 * @brief Get username.
 * 
 * @return const string& : Username
 */
const string& User::getUsername(void) const {
    return _username;
}

/**
 * @brief Verify that the user is authenticated.
 * 
 * @return true : Is authenticated(PASS, NICK, USER commands processed) / if not return
 * @return false 
 */
bool User::getAuth(void) const {
    return _auth;
}

/**
 * @brief Gets the cmd buffer for that user.
 *  The cmd buffer is a message sent by that user to the server.
 * 
 * @return const string& : Contents of cmd buffer
 */
const string& User::getCmdBuffer(void) const {
    return _cmdBuffer;
}

/**
 * @brief Gets the reply buffer for that user.
 *  The reply buffer is a message that the server will send to the user.
 * 
 * @return const string& : Contents of reply buffer
 */
const string& User::getReplyBuffer(void) const {
    return _replyBuffer;
}

/**
 * @brief Get all channels to which that user belongs.
 * 
 * @return const vector<Channel *>& : The channels to which the user belongs.
 */
const vector<Channel *>& User::getMyAllChannel(void) const {
    return _myChannelList;
}

/**
 * @brief Verify that the user is disconnecting to server.
 * 
 * @return true User is disconnecting / if not return
 * @return false 
 */
bool User::getIsQuiting(void) const {
    return _isQuiting;
}

/**
 * @brief Record the password that the user passed by the PASS command.
 * The actual verification process takes place after NICK, USER commands are processed.
 * 
 * @param pwd Password that the user passed by the PASS command.
 */
void User::setPassword(const string& pwd) {
    _password = pwd;
}

/**
 * @brief Set the user requested nickname.
 * 
 * @param nickname Requested nickname that the user passed by the NICK command.
 */
void User::setNickname(const string& nickname) {
    _nickname = nickname;
}

/**
 * @brief Set the username.
 * 
 * @param nickname Username that the user passed by the USER command.
 */
void User::setUsername(const string& username) {
    _username = username;
}

/**
 * @brief Authenticate the user.
 */
void User::setAuth(void) {
    _auth = true;
}

/**
 * @brief Replace the cmd buffer with the given string value.
 * 
 * @param str 
 */
void User::setCmdBuffer(const string& str) {
    _cmdBuffer = str;
}

/**
 * @brief Empty the cmd buffer of the user
 */
void User::clearCmdBuffer(void) {
    _cmdBuffer.clear();
}

/**
 * @brief Replace the reply buffer with the given string value.
 * 
 * @param str 
 */
void User::setReplyBuffer(const string& str) {
    _replyBuffer = str;
}

/**
 * @brief Replace the reply buffer with the given Message instance.
 *  Set to the return value of the createReplyForm() for that message instance.
 * @param msg 
 */
void User::setReplyBuffer(const Message& msg) {
    _replyBuffer = msg.createReplyForm();
}

/**
 * @brief Empty the reply buffer of the user
 */
void User::clearReplyBuffer(void) {
    _replyBuffer.clear();
}

/**
 * @brief Adds the given string after the existing cmd buffer.
 * 
 * @param str 
 */
void User::addToCmdBuffer(const string& str) {
    _cmdBuffer.append(str);
}

/**
 * @brief Adds the given string after the existing reply buffer.
 * 
 * @param str 
 */
void User::addToReplyBuffer(const string& str) {
    _replyBuffer.append(str);
}

/**
 * @brief Adds the given Message after the existing reply buffer.
 *  Add return value of the createReplyForm() for that message instance.
 * 
 * @param msg 
 */
void User::addToReplyBuffer(const Message& msg) {
    _replyBuffer.append(msg.createReplyForm());
}

/**
 * @brief When user enter a new channel, add it to the user's channel list.
 * 
 * @param channel : Pointer of the participating channel instance
 */
void User::addToMyChannelList(Channel *channel) {
    _myChannelList.push_back(channel);
}

/**
 * @brief When user leave a channel, delete it from the channel list for that user.
 * 
 * @param channel : Pointer of the channel instance that left
 */
void User::deleteFromMyChannelList(Channel *channel) {
    vector<Channel *>::iterator it = find(_myChannelList.begin(), _myChannelList.end(), channel);

    if (it == _myChannelList.end()) return ;

    _myChannelList.erase(it);
}

/**
 * @brief Empty the list of channels to which the user belongs.
 */
void User::clearMyChannelList(void) {
    _myChannelList.clear();
}

/**
 * @brief Send messages to the channels to which the user belongs.
 * 
 * @param msg Message to send
 * @param ignoreFd Client socket fd not to be sent. Own fd are set as the default parameter.
 */
void User::broadcastToMyChannels(const Message& msg, const int ignoreFd) const {
    const vector<Channel *>& chs = getMyAllChannel();

	for (vector<Channel *>::const_iterator it = chs.begin(); it != chs.end(); ++it) {
		(*it)->broadcast(msg, ignoreFd);
	}
}

/**
 * @brief Indicates that the user is leaving the server.
 *  Set in the QUIT command.
 */
void User::setIsQuiting(void) {
    _isQuiting = true;
}
