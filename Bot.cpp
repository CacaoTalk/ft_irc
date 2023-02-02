#include "Bot.hpp"

Bot::Bot() {
	srand(time(NULL));
}

Bot::~Bot() {
	_menuList.clear();
}

void Bot::addMenu(vector<string> params) {
	for (vector<string>::size_type i=1; i<params.size(); i++) {
		_menuList.insert(params[i]);
	}
}

void Bot::deleteMenu(vector<string> params) {
	for (vector<string>::size_type i=1; i<params.size(); i++) {
		_menuList.erase(params[i]);
	}
}

const string Bot::showMenu(void) const {
	string reply;
	set<string>::const_iterator it;

	reply = "MENU : ";
	for (it = _menuList.begin(); it != _menuList.end(); ++it) {
		if (it != _menuList.begin()) reply += ", ";
		reply += (*it);
	}
	return reply;
}

const string Bot::pickMenu(void) const {
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
