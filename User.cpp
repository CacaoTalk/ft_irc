#include "User.hpp"

User::User(int fd) : _fd(fd) { }

User::~User() { }

string User::getNickname(void) const {
    return _nickname;
}

string User::getCmdBuffer(void) {
    return _cmdBuffer;
}

string User::getReplyBuffer(void) {
    return _replyBuffer;
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