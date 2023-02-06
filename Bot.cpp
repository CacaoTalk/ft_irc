#include "Bot.hpp"
/**
 * @brief Construct a new Bot:: Set random seed
 */
Bot::Bot() {
	srand(time(NULL));
}

/**
 * @brief Destroy the Bot:: Remove menu list
 */
Bot::~Bot() {
	_menuList.clear();
}

/**
 * @brief Add menus to list<set>
 * 
 * @param params menu names
 */
void Bot::addMenu(vector<string> params) {
	for (vector<string>::size_type i=1; i<params.size(); i++) {
		_menuList.insert(params[i]);
	}
}

/**
 * @brief Delete menus from list<set>
 * 
 * @param params menu names
 */
void Bot::deleteMenu(vector<string> params) {
	for (vector<string>::size_type i=1; i<params.size(); i++) {
		_menuList.erase(params[i]);
	}
}

/**
 * @brief Make menu list string from current list
 * 
 * @return const string "MENU : <menu1>, <menu2>..."
 */
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

/**
 * @brief Choose one menu from current list
 * 
 * @return const string "I recommend <menu>"
 */
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
