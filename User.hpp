#pragma once

#ifndef USER_HPP
# define USER_HPP

# include <string>
# include <vector>

# include "CommonValue.hpp"

using namespace std;

class Channel;
class Message;
class User {
	private:
		int _fd;
		string _host;
		string _password;
		string _nickname; // unique
		string _username;
		bool _auth;
		string _cmdBuffer;
		string _replyBuffer;
		vector<Channel *> _myChannelList;
		bool _isQuiting;

		User(void);
		User(const User& user);
		User& operator=(const User& user);

	public:
        User(int fd, const string& host);
		~User();
		
		int getFd(void) const;
		const string& getHost(void) const;
		const string& getPassword(void) const;
		const string getNickname(void) const;
		const string getSource(void) const;
		const string& getUsername(void) const;
		bool getAuth(void) const;
		const string& getCmdBuffer(void) const;
		const string& getReplyBuffer(void) const;
		const vector<Channel *>& getMyAllChannel(void) const;
		bool getIsQuiting(void) const;

		void setPassword(const string& pwd);
		void setNickname(const string& nickname);
		void setUsername(const string& username);
		void setAuth(void);
		void setIsQuiting(void);

		void setCmdBuffer(const string& src);
		void clearCmdBuffer(void);
		void setReplyBuffer(const string& src);
		void setReplyBuffer(const Message& msg);
		void clearReplyBuffer(void);
		void addToCmdBuffer(const string& src);
		void addToReplyBuffer(const string& src);
		void addToReplyBuffer(const Message& msg);

		void addToMyChannelList(Channel* channel);
		void deleteFromMyChannelList(Channel* channel);
		void clearMyChannelList(void);
		void broadcastToMyChannels(const Message& msg, const int ignoreFd = UNDEFINED_FD) const;

};

#endif
