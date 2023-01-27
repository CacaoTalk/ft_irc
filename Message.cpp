#include "Server.hpp"
// for debug
#include <iostream>

Message::Message(void) {}

Message::Message(const string& msg) {
    parse(msg);
    // for debug: print command and params
    cout << "COMMAND: " << _command << endl;
    for (vector<string>::iterator it = _params.begin(); it != _params.end(); ++it) {
        cout <<"PARAMS: " << *it << endl;
    }
}

Message::~Message() {
    _command.clear();
    _params.clear();
}

const string Message::createReplyForm(void) {
    string reply;

    vector<string>::iterator it;
    for (it = _params.begin(); it != _params.end(); ++it) {
        if (it != _params.begin() && reply != ":") reply += " ";
        reply += (*it);
    }
    reply += "\r\n";
    return reply;
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
    splited.push_back(str.substr(cursorPos, string::npos));
    return splited;
}

void Message::parse(const string& msg) {
    vector<string> splitedBySpace = split(msg, ' ');

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

string Message::getCommand() const {
    return _command;
}

vector<string> Message::getParams() const {
    return _params;
}

Message& Message::operator<<(const string param) {
    _params.push_back(param);
    return (*this);
}