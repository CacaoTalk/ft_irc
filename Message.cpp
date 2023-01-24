#include "Message.hpp"

Message::Message(const string& msg) {
    // parse();
	// selectCommand();
}

Message::~Message() {
    _commands.clear();
    _params.clear();
}
