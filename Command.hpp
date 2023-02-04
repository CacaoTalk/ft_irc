#pragma once

#ifndef COMMAND_HPP
# define COMMAND_HPP

# include <map>
# include <string>

using namespace std;

class Server;
class User;
class Message;

class Command {
	typedef bool (Command::*ircCommand)(User *, const Message&);

	private:
		Server& _server;
		map<string, ircCommand> _commands;

		Command(void);
		Command(const Command& src);
		Command& operator=(const Command& src);

		bool cmdPrivmsg(User *user, const Message& msg);
		bool cmdJoin(User *user, const Message& msg);
		bool cmdPart(User *user, const Message& msg);
		bool cmdPass(User *user, const Message& msg);
		bool cmdNick(User *user, const Message& msg);
		bool cmdUser(User *user, const Message& msg);
		bool cmdPing(User *user, const Message& msg);
		bool cmdQuit(User *user, const Message& msg);
		bool cmdKick(User *user, const Message& msg);
		bool cmdNotice(User *user, const Message& msg);

	public:
		Command(Server& server);
		~Command();

		bool run(User *user, const Message& msg);
};

#endif