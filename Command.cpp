#include "Command.hpp"

#include "Server.hpp"
#include "User.hpp"
#include "Message.hpp"

#include "Reply.hpp"

#include <iostream>

bool Command::runCommand(Server& server, User *user, const Message& msg) {
	const string& cmd = msg.getCommand();
    if (cmd == "PRIVMSG") return cmdPrivmsg(server, user, msg);
    else if (cmd == "JOIN") return cmdJoin(server, user, msg);
    else if (cmd == "PART") return cmdPart(server, user, msg);
	else if (cmd == "PASS") return cmdPass(user, msg);
	else if (cmd == "NICK") return cmdNick(server, user, msg);
	else if (cmd == "USER") return cmdUser(server, user, msg);
	else if (cmd == "PING") return cmdPing(user, msg);
	else if (cmd == "QUIT") return cmdQuit(server, user, msg); 
	else if (cmd == "KICK") return cmdKick(server, user, msg);
	else if (cmd == "NOTICE") return cmdNotice(server, user, msg);
	return true;
}

bool Command::cmdPrivmsg(Server& server, User *user, const Message& msg) {
    if (msg.paramSize() < 2) {
		Message replyMsg;
		replyMsg << ":" << SERVER_HOSTNAME << ERR_NORECIPIENT << user->getNickname() << ERR_NORECIPIENT_MSG << "(PRIVMSG)";
		user->addToReplyBuffer(replyMsg);
		return true;
	}

    const vector<string> targetList = Message::split(msg[0], ',');
    for (vector<string>::const_iterator it = targetList.begin(); it != targetList.end(); ++it) {
        string targetName = *it;
        if (targetName[0] == '#') {
            Channel *targetChannel;

            targetChannel = server.findChannelByName(targetName);
            if (targetChannel == NULL) {
				Message replyMsg;
				replyMsg << ":" << SERVER_HOSTNAME << ERR_NOSUCHNICK << user->getNickname() << targetName << ERR_NOSUCHNICK_MSG;
				user->addToReplyBuffer(replyMsg);
				continue;
			}
			Message replyMsg;
			replyMsg << ":" << user->getNickname() << msg.getCommand() << targetChannel->getName() << msg[1];
            targetChannel->broadcast(replyMsg, user->getFd());
        } else {
            User *targetUser;

            targetUser = server.findClientByNickname(targetName);
            if (targetUser == NULL) {
				Message replyMsg;
				replyMsg << string(":").append(SERVER_HOSTNAME) << ERR_NOSUCHNICK << user->getNickname() << targetName << ERR_NOSUCHNICK_MSG;
				user->addToReplyBuffer(replyMsg);
				continue;
			}
			Message replyMsg;
			replyMsg << ":" << user->getNickname() << msg.getCommand() << targetUser->getNickname() << msg[1];
            targetUser->addToReplyBuffer(replyMsg);
        }
    }
	return true;
}

bool Command::cmdJoin(Server& server, User *user, const Message& msg) {
    if (msg.paramSize() == 0) {
		Message replyMsg;
		replyMsg << ":" << SERVER_HOSTNAME << ERR_NEEDMOREPARAMS << user->getNickname() << msg.getCommand() << ERR_NEEDMOREPARAMS_MSG;
		user->addToReplyBuffer(replyMsg);
		return true;
	}
    
    const vector<string> targetList = Message::split(msg[0], ',');
    if (targetList.size() == 1 && targetList[0] == "0") {
        vector<string> removeWaitingChannels;
        cout << user->getNickname() << " LEAVE FROM ALL CHANNELS" << endl;
		const map<string, Channel *>& chs = server.getAllChannel();
        for (map<string, Channel *>::const_iterator it = chs.begin(); it != chs.end(); ++it) {
			Channel *targetChannel = it->second;
			User *userInChannel = targetChannel->findUser(user->getFd());

			if (userInChannel == NULL) continue;
			
            const int remainUserOfChannel = targetChannel->deleteUser(user->getFd());
			Message replyMsg;
			replyMsg << user->getNickname() << "PART" << targetChannel->getName();
			user->addToReplyBuffer(replyMsg);
			targetChannel->broadcast(replyMsg);
            if (remainUserOfChannel == 0) removeWaitingChannels.push_back(targetChannel->getName());
        }
        for (vector<string>::iterator it = removeWaitingChannels.begin(); it != removeWaitingChannels.end(); ++it) {
            server.deleteChannel(*it);
        }
        return true;
    }

    for (vector<string>::const_iterator it = targetList.begin(); it != targetList.end(); ++it) {
        string targetChannelName = *it;
        if (targetChannelName[0] != '#') {
			Message replyMsg;
			replyMsg << ":" << SERVER_HOSTNAME << ERR_NOSUCHCHANNEL << user->getNickname() << targetChannelName << ERR_NOSUCHCHANNEL_MSG;
			user->addToReplyBuffer(replyMsg);
			continue;
		}

        Channel *targetChannel;

        targetChannel = server.findChannelByName(targetChannelName);
        if (targetChannel == NULL) {
            targetChannel = server.addChannel(targetChannelName);
        } else if (targetChannel->findUser(user->getFd()) != NULL) continue;
		
        targetChannel->addUser(user->getFd(), user);
		Message replyMsg[3];
		replyMsg[0] << ":" << user->getNickname() << msg.getCommand() << ":" << targetChannelName;
		replyMsg[1] << ":" << SERVER_HOSTNAME << RPL_NAMREPLY << user->getNickname() << "=" << targetChannelName << ":";
		vector<string> targetChannelUserList = targetChannel->getUserList();
		for (vector<string>::iterator it = targetChannelUserList.begin(); it != targetChannelUserList.end(); ++it) {
			replyMsg[1] << *it;
		}
		replyMsg[2] << ":" << SERVER_HOSTNAME << RPL_ENDOFNAMES << user->getNickname() << targetChannelName << RPL_ENDOFNAMES_MSG;
		targetChannel->broadcast(replyMsg[0]);
		user->addToReplyBuffer(replyMsg[1]);		
		user->addToReplyBuffer(replyMsg[2]);
    }
	return true;
}

bool Command::cmdPart(Server& server, User *user, const Message& msg) {
	if (msg.paramSize() < 1) {
		Message replyMsg;
		replyMsg << ":" << SERVER_HOSTNAME << ERR_NEEDMOREPARAMS << user->getNickname() << msg.getCommand() << ERR_NEEDMOREPARAMS_MSG;
		user->addToReplyBuffer(replyMsg);
		return true;
	}

    string partNotiMessage;
    if (msg.paramSize() >= 2) {
		partNotiMessage.append(":");
		partNotiMessage.append(msg[1]);
	}

    const vector<string> targetList = Message::split(msg[0], ',');
    for (vector<string>::const_iterator it = targetList.begin(); it != targetList.end(); ++it) {
        string targetChannelName = *it;

        Channel *targetChannel;
        targetChannel = server.findChannelByName(targetChannelName);

        if (targetChannel == NULL) {
			Message replyMsg;
			replyMsg << ":" << SERVER_HOSTNAME << ERR_NOSUCHCHANNEL << user->getNickname() << targetChannelName << ERR_NOSUCHCHANNEL_MSG;
			user->addToReplyBuffer(replyMsg);
			continue;
		}
		if (targetChannel->findUser(user->getFd()) == NULL) {
			Message replyMsg;
			replyMsg << ":" << SERVER_HOSTNAME << ERR_NOTONCHANNEL << user->getNickname() << targetChannelName << ERR_NOTONCHANNEL_MSG;
			user->addToReplyBuffer(replyMsg);
			continue;
		}
        const int remainUserOfChannel = targetChannel->deleteUser(user->getFd());
		Message replyMsg;
		replyMsg << user->getNickname() << "PART" << targetChannelName;
		user->addToReplyBuffer(replyMsg);
		targetChannel->broadcast(replyMsg);
        if (remainUserOfChannel == 0) server.deleteChannel(targetChannelName);
    }
	return true;
}

bool Command::cmdPass(User *user, const Message& msg) {
	Message replyMsg;
	
	if (msg.paramSize() < 1) {
		replyMsg << ":" << SERVER_HOSTNAME << ERR_NEEDMOREPARAMS << user->getNickname() << msg.getCommand() << ERR_NEEDMOREPARAMS_MSG;
		user->addToReplyBuffer(replyMsg);
		return true;
	}
	if (user->getAuth()) {
		replyMsg << ":" << SERVER_HOSTNAME << ERR_ALREADYREGISTERED << user->getNickname() << ERR_ALREADYREGISTERED_MSG;
		user->addToReplyBuffer(replyMsg);
		return true;
	}
	user->setPassword(msg[0]);
	return true;
}

bool Command::cmdNick(Server& server, User *user, const Message& msg) {
	Message replyMsg;
	
	if (msg.paramSize() < 1) {
		replyMsg << ":" << SERVER_HOSTNAME << ERR_NEEDMOREPARAMS << user->getNickname() << msg.getCommand() << ERR_NEEDMOREPARAMS_MSG;
		user->addToReplyBuffer(replyMsg);
		return true;
	}
	const string requestNickname = msg[0];
	const string originNickname = user->getNickname();

	if (requestNickname.length() == 0) {
		replyMsg << ":" << SERVER_HOSTNAME << ERR_NONICKNAMEGIVEN << originNickname << ERR_NONICKNAMEGIVEN_MSG;
		user->addToReplyBuffer(replyMsg);
		return true;
	}

	if (server.findClientByNickname(requestNickname) != NULL) {
		replyMsg << ":" << SERVER_HOSTNAME << ERR_NICKNAMEINUSE << originNickname << requestNickname << ERR_NICKNAMEINUSE_MSG;
		user->addToReplyBuffer(replyMsg);
		return true;
	}
	
	user->setNickname(requestNickname);
	if (!user->getAuth() && !user->getUsername().empty()) {
		if (server.checkPassword(user->getPassword())) {
			user->setAuth();
			replyMsg << ":" << SERVER_HOSTNAME << RPL_WELCOME << user->getNickname() << ":Welcome to the" << SERVER_HOSTNAME <<  "Network" << requestNickname;
			user->addToReplyBuffer(replyMsg);
			return true;
		}
		else {
			replyMsg << ":" << SERVER_HOSTNAME << ERR_PASSWDMISMATCH << user->getNickname() << ERR_PASSWDMISMATCH_MSG;
			user->addToReplyBuffer(replyMsg);
			server.disconnectClient(user->getFd());
			return false;
		}
	}
	replyMsg << ":" << originNickname << msg.getCommand() << requestNickname;
	user->addToReplyBuffer(replyMsg);
	return true;
}

bool Command::cmdUser(Server& server, User *user, const Message& msg) {
	Message replyMsg;
	
	if (msg.paramSize() < 4) {
		replyMsg << ":" << SERVER_HOSTNAME << ERR_NEEDMOREPARAMS << user->getNickname() << msg.getCommand() << ERR_NEEDMOREPARAMS_MSG;
		user->addToReplyBuffer(replyMsg);
		return true;
	}
	
	if (user->getAuth()) { // 이미 auth되어 있는 user
		replyMsg << ":" << SERVER_HOSTNAME << ERR_ALREADYREGISTERED << user->getNickname() << ERR_ALREADYREGISTERED_MSG;
		user->addToReplyBuffer(replyMsg);
		return true;
	}
	
	const string requestUserNickname = msg[0]; //username
	
	if (requestUserNickname.length() == 0) {
		replyMsg << ":" << SERVER_HOSTNAME << ERR_NEEDMOREPARAMS << user->getNickname() << msg.getCommand() << ERR_NEEDMOREPARAMS_MSG;
		user->addToReplyBuffer(replyMsg);
		return true;
	}
	
	user->setUsername(requestUserNickname);
	if (!user->getNickname().empty()) {
		if (server.checkPassword(user->getPassword())) {
			user->setAuth();
			replyMsg << ":" << SERVER_HOSTNAME << RPL_WELCOME << user->getNickname() << ":Welcome to the" << SERVER_HOSTNAME <<  "Network" << requestUserNickname;
			user->addToReplyBuffer(replyMsg);
			return true;
		}
		else {
			replyMsg << ":" << SERVER_HOSTNAME << ERR_PASSWDMISMATCH << user->getNickname() << ERR_PASSWDMISMATCH_MSG;
			user->addToReplyBuffer(replyMsg);
			server.disconnectClient(user->getFd());
			return false;
		}
	}
	return true;
}

bool Command::cmdPing(User *user, const Message& msg) {
	Message replyMsg;

	if (msg.paramSize() < 1) {
		replyMsg << ":" << SERVER_HOSTNAME << ERR_NEEDMOREPARAMS << user->getNickname() << msg.getCommand() << ERR_NEEDMOREPARAMS_MSG;
		user->addToReplyBuffer(replyMsg);
		return true;
	}

	if (msg[0].empty()) {
		replyMsg << ":" << SERVER_HOSTNAME << ERR_NOORIGIN << user->getNickname() << ERR_NOORIGIN_MSG;
		user->addToReplyBuffer(replyMsg);
		return true;
	}
	
	replyMsg << ":" << SERVER_HOSTNAME << "PONG" << SERVER_HOSTNAME << msg[0];
	user->addToReplyBuffer(replyMsg);
	return true;
}

bool Command::cmdQuit(Server& server, User *user, const Message& msg) {
	if (msg.paramSize() < 1) {
		Message replyMsg;

		replyMsg << ":" << SERVER_HOSTNAME << ERR_NEEDMOREPARAMS << user->getNickname() << msg.getCommand() << ERR_NEEDMOREPARAMS_MSG;
		user->addToReplyBuffer(replyMsg);
		return true;
	}

	string reason = ":Quit:";
	if (msg.paramSize() == 1) reason += msg[0];
	else reason += "leaving";
	
	Message replyMsg;
	replyMsg << ":" << user->getNickname() << msg.getCommand() << reason;
	
	int clientFd = user->getFd();
	const map<string, Channel *>& chs = server.getAllChannel();
	map<string, Channel *>::const_iterator it;
	for (it = chs.begin(); it != chs.end(); ++it) {
		if (it->second->findUser(clientFd) != NULL) {
			it->second->broadcast(replyMsg, clientFd);
		}
	}
	server.disconnectClient(clientFd);
	return false;
}

// 
bool Command::cmdKick(Server& server, User *user, const Message& msg) {
	if (msg.paramSize() < 2) {
		Message replyMsg;
		
		replyMsg << ":" << SERVER_HOSTNAME << ERR_NEEDMOREPARAMS << user->getNickname() << msg.getCommand() << ERR_NEEDMOREPARAMS_MSG;
		user->addToReplyBuffer(replyMsg);
	}

	string reason;
	if (msg.paramSize() >= 3) {
		reason.append(":");
		reason.append(msg[2]);
	}
	
	// 해당 channel이 존재하는 지 check
	Channel *targetChannel = server.findChannelByName(msg[0]);
	if (targetChannel == NULL) {
		Message replyMsg;

		replyMsg << ":" << SERVER_HOSTNAME << ERR_NOSUCHCHANNEL << user->getNickname() << msg[0] << ERR_NOSUCHCHANNEL_MSG;
		user->addToReplyBuffer(replyMsg);
		return true;
	}
	
	// User가 channel에 있는 지 check
	if (targetChannel->findUser(user->getFd()) == NULL) {\
		Message replyMsg;

		replyMsg << ":" << SERVER_HOSTNAME << ERR_NOTONCHANNEL << user->getNickname() << msg[0] << ERR_NOTONCHANNEL_MSG;
		user->addToReplyBuffer(replyMsg);
		return true;
	}

	// User가 해당 channel의 operator인지
	if (targetChannel->isUserOper(user->getFd()) == false) {
		Message replyMsg;

		replyMsg << ":" << SERVER_HOSTNAME << ERR_CHANOPRIVSNEEDED << user->getNickname() << msg[0] << ERR_CHANOPRIVSNEEDED_MSG;
		user->addToReplyBuffer(replyMsg);
		return true;
	}

	// iteration
	const vector<string> targetUsers = Message::split(msg[1], ',');
	for (vector<string>::const_iterator it = targetUsers.begin(); it != targetUsers.end(); ++it) {
		// target User가 channel에 존재하는지
		int targetFd = server.findClientByNickname(*it)->getFd();
		if (targetChannel->findUser(targetFd) == NULL) {
			Message replyMsg;
			replyMsg << ":" << SERVER_HOSTNAME << ERR_USERNOTINCHANNEL << user->getNickname() << *it << msg[0] << ERR_USERNOTINCHANNEL_MSG;
			user->addToReplyBuffer(replyMsg);
		}
		// 존재하면 Kick (그 channel에 deleteUser)
		Message replyMsg;
		
		replyMsg << ":" << user->getNickname() << msg.getCommand() << msg[0] << *it << ":" << user->getNickname();
		targetChannel->broadcast(replyMsg);
		const int remainUsers = targetChannel->deleteUser(targetFd);
		if (remainUsers == 0) server.deleteChannel(targetChannel->getName());
	}
	return true;
}

bool Command::cmdNotice(Server& server, User *user, const Message& msg) {
	Message replyMsg;
	
	if (msg.paramSize() == 0) {
		replyMsg << ":" << SERVER_HOSTNAME << ERR_NORECIPIENT << user->getNickname() << ERR_NORECIPIENT_MSG << "(NOTICE)";
		user->addToReplyBuffer(replyMsg);
		return true;
	}
	if (msg.paramSize() == 1) {
		replyMsg << ":" << SERVER_HOSTNAME << ERR_NOTEXTTOSEND << user->getNickname() << msg.getCommand() << ERR_NOTEXTTOSEND_MSG;
		user->addToReplyBuffer(replyMsg);
		return true;
	}

    const vector<string> targetList = Message::split(msg[0], ',');
    for (vector<string>::const_iterator it = targetList.begin(); it != targetList.end(); ++it) {
        string targetName = *it;
        if (targetName[0] == '#') {
            Channel *targetChannel;

            targetChannel = server.findChannelByName(targetName);
            if (targetChannel == NULL) continue;
			// FIXME: IRC Message format?
            // targetChannel->broadcast(msg[1] + '\n', user->getFd());
        } else {
            User *targetUser;

            targetUser = server.findClientByNickname(targetName);
            if (targetUser == NULL) continue;
			// FIXME: IRC Message format?
            // targetUser->addToReplyBuffer(msg[1] + '\n'); // Format.. 
        }
    }
	return true;
}
