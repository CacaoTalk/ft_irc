#include "Message.hpp"

Message::Message(const string& msg) {
    parse(msg);
	// selectCommand();
}

Message::~Message() {
    _commands.clear();
    _params.clear();
}

void Message::parse(const string& msg) {
    size_t cursorPos = 0;
    size_t spacePos;

    spacePos = msg.find(' ', cursorPos);
    if (spacePos == string::npos) {
        _commands = msg.substr(cursorPos, string::npos);
        return;
    }
    _commands = msg.substr(cursorPos, spacePos - cursorPos);

    while (true) {
        cursorPos = spacePos;
        while (msg.at(cursorPos) == ' ') ++cursorPos;

        if (msg.at(cursorPos) == ':') {
            _params.push_back(msg.substr(cursorPos + 1, string::npos));
            return ;
        }

        spacePos = msg.find(' ', cursorPos);
        if (spacePos == string::npos) break;

        _params.push_back(msg.substr(cursorPos, spacePos - cursorPos));
    }
    _params.push_back(msg.substr(cursorPos, string::npos));
}
