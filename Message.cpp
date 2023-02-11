#include "Server.hpp"

/**
 * @brief Construct a new Message:: Create empty message class for make reply
 */
Message::Message(void) {}

/**
 * @brief Construct a new Message:: Create message class with received message from
 *  client. Parse to the IRC message rules.
 * 
 * @param ircMsgFormStr : String of IRC format message that received from client
 */
Message::Message(const string& ircMsgFormStr) {
    parse(ircMsgFormStr);
}

/**
 * @brief Destroy the Message:: Message object
 */
Message::~Message() {
    _command.clear();
    _params.clear();
}

/**
 * @brief Parse strings received from clients to IRC message format.
 * 
 * @param ircMsgFormStr strings received from clients
 */
void Message::parse(const string& ircMsgFormStr) {
    vector<string> splitedBySpace = split(ircMsgFormStr, ' ');

    for (vector<string>::size_type i = 0; i < splitedBySpace.size(); ++i) {
        if (i == 0 && splitedBySpace[i][0] == ':') {
            _prefix = splitedBySpace[i].erase(0, 1);
            continue ;
        }
        if (_command.empty()) {
            _command = splitedBySpace[i];
            continue;
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

/**
 * @brief Get prefix(deinfed in IRC RFC doc) from a parsed message.
 * 
 * @return const string& : prefix(Most of the time, it's empty or nickname)
 */
const string& Message::getPrefix(void) const {
    return _prefix;
}

/**
 * @brief Get command(defined in IRC RFC doc) from a parsed message.
 * 
 * @return const string& : command(ex. PRIVMSG, JOIN etc...)
 */
const string& Message::getCommand(void) const {
    return _command;
}

/**
 * @brief Get params from a parsed message. These are related to the command.
 * 
 * @return const vector<string>& : params
 */
const vector<string>& Message::getParams(void) const {
    return _params;
}

/**
 * @brief Separate the string based on the delimiter given. 
 *  Space is used when parsing IRC messages, and ',' is used when parsing parameters.
 * 
 * @param str String to be separated. Most are IRC messages or parameter strings.
 * @param delimeter The characters that will be used as the basis for separation criteria
 * @return vector<string> : Data structure with separated strings
 */
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

/**
 * @brief Returns the number of parameters that the message has.
 * 
 * @return size_t : Number of parameters
 */
size_t Message::paramSize(void) const {
    return _params.size();
}

/**
 * @brief Connect the parameters you have to create a reply that matches the IRC message rule. 
 *  This function is usually run after creating a new Message instance and 
 *  inserting the required parameters.
 * 
 * @return const string : A string associated with the parameters that the message instance has.
 *  (ex. ":<hostname> <numeric_reply> <nickname> :<error_msg>")
 */
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

/**
 * @brief Add to params. Use it in the order in which it appears in the reply message.
 *  If you put a single ":" to params, 
 *  it is created by attaching it to the following parameters when createReplyForm().
 * 
 * @param param Reply message element(ex. source, numeric reply, msg, etc...)
 * @return Message& 
 */
Message& Message::operator<<(const string param) {
    if (!param.empty()) _params.push_back(param);
    return (*this);
}
