#include "Server.hpp"

Message::Message(void) {}

Message::Message(const string& ircMsgFormStr) {
    parse(ircMsgFormStr);
}

Message::~Message() {
    _command.clear();
    _params.clear();
}

void Message::parse(const string& ircMsgFormStr) {
    vector<string> splitedBySpace = split(ircMsgFormStr, ' ');

    for (vector<string>::size_type i = 0; i < splitedBySpace.size(); ++i) {
        if (i == 0) {
            _command = splitedBySpace[i];
            continue ;
        }
        if (splitedBySpace[i][0] == ':') {
            string mergedString;
            splitedBySpace[i].erase(0, 1);
            while (i < splitedBySpace.size()) {
                mergedString += splitedBySpace[i];
                if (i != splitedBySpace.size() - 1) mergedString += ' ';
                ++i;
            }
            _params.push_back(mergedString);
            return ;
        } else _params.push_back(splitedBySpace[i]);
    }
}
const string& Message::getCommand(void) const {
    return _command;
}

const vector<string>& Message::getParams(void) const {
    return _params;
}

vector<string> Message::split(const string& str, const char delimeter) {
    vector<string> splited;
    size_t cursorPos = 0;
    size_t delimeterPos;

    while ((delimeterPos = str.find(delimeter, cursorPos)) != string::npos) {
        splited.push_back(str.substr(cursorPos, delimeterPos - cursorPos));
        while (str.at(delimeterPos) == delimeter) {
            if (++delimeterPos == str.length()) return splited;
        }
        cursorPos = delimeterPos;
    }
    splited.push_back(str.substr(cursorPos));
    return splited;
}

size_t Message::paramSize(void) const {
    return _params.size();
}

const string Message::createReplyForm(void) const {
    string reply;

    vector<string>::const_iterator it;
    for (it = _params.begin(); it != _params.end(); ++it) {
        reply += (*it);
        if (*it != ":" && (it + 1) != _params.end()) reply += ' ';
    }
    reply += "\r\n";
    return reply;
}

Message& Message::operator<<(const string param) {
    if (!param.empty()) _params.push_back(param);
    return (*this);
}
