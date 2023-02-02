#include "Bot.hpp"

Bot::Bot() {
	srand(time(NULL));
}

Bot::~Bot() {

}


void Bot::addMenu(const string& menu) {
	_menuList.insert(menu);
}

void Bot::delteMenu(const string& menu) {
	_menuList.erase(menu);
}

const string Bot::showMenu(void) const {
	string reply;
	set<string>::const_iterator it;

	reply = "MENU : ";
	for (it = _menuList.begin(); it != _menuList.end(); ++it) {
		reply += (*it) + ", ";
	}
	return reply;
}

const string Bot::pickMenu(const set<string> menu) const {
	if (_menuList.empty()) {
		return ("Empty List");
	}

	set<string>::iterator it = _menuList.begin();
	int pickedMenuIdx = rand() % _menuList.size();
	while (pickedMenuIdx--) {
		it++;
	}
	const string pickedMenu = *it;
	string reply = "I recommend ";
	reply += pickedMenu;
	return reply;
}
