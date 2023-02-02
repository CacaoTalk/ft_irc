#pragma once

#ifndef COMMAND_HPP
# define COMMAND_HPP

class Server;
class User;
class Message;

struct Command
{
	static bool runCommand(Server& server, User *user, const Message& msg);

	static bool cmdPrivmsg(Server& server, User *user, const Message& msg);
	static bool cmdJoin(Server& server, User *user, const Message& msg);
	static bool cmdPart(Server& server, User *user, const Message& msg);
	static bool cmdPass(User *user, const Message& msg);
	static bool cmdNick(Server& server, User *user, const Message& msg);
	static bool cmdUser(Server& server, User *user, const Message& msg);
	static bool cmdPing(User *user, const Message& msg);
	static bool cmdQuit(Server& server, User *user, const Message& msg);
	static bool cmdKick(Server& server, User *user, const Message& msg);
	static bool cmdNotice(Server& server, User *user, const Message& msg);
};

#endif