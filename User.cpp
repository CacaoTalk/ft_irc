#include "User.hpp"

User::User(int fd) : _fd(fd), _auth(false) { }

User::~User() { }

int User::getFd(void) {
    return _fd;
}

string User::getPassword(void) const {
    return _password;
}

string User::getNickname(void) const {
    if (_nickname.empty()) return "*";
    
    return _nickname;
}

string User::getUsername(void) const {
    return _username;
}

bool User::getAuth(void) const {
    return _auth;
}

string User::getCmdBuffer(void) {
    return _cmdBuffer;
}

const string User::getCmdBuffer(void) const {
    return _cmdBuffer;
}

string User::getReplyBuffer(void) {
    return _replyBuffer;
}

const string User::getReplyBuffer(void) const {
    return _replyBuffer;
}

void User::setPassword(const string& pwd) {
    _password = pwd;
}

void User::setNickname(const string& nickname) {
    _nickname = nickname;
}

void User::setUsername(const string& username) {
    _username = username;
}

void User::setAuth() {
    _auth = true;
}

void User::setCmdBuffer(const string& str) {
    _cmdBuffer = str;
}

void User::setReplyBuffer(const string& str) {
    _replyBuffer = str;
}

void User::addToCmdBuffer(const string& str) {
    _cmdBuffer.append(str);
}

void User::addToReplyBuffer(const string& str) {
    _replyBuffer.append(str);
}