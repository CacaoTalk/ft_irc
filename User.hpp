#pragma once

#ifndef USER_HPP
# define USER_HPP

# include <string>
# include <vector>
using namespace std;

class Channel;
class Message;
class User {
	private:
		int _fd;
		string _password;
		string _nickname; // unique
		string _username;
		bool _auth;
		string _cmdBuffer;
		string _replyBuffer;
		vector<Channel *> _myChannelList;

		User(void);
		User(const User& user);
		User& operator=(const User& user);

	public:
        User(int fd);
		~User();
		
		int getFd(void);
		const string& getPassword(void) const;
		const string getNickname(void) const;
		const string& getUsername(void) const;
		bool getAuth(void) const;
		const string& getCmdBuffer(void) const;
		const string& getReplyBuffer(void) const;
		const vector<Channel *>& getMyAllChannel(void) const;

		void setPassword(const string& pwd);
		void setNickname(const string& nickname);
		void setUsername(const string& username);
		void setAuth(void);

		void setCmdBuffer(const string& src);
		void setReplyBuffer(const string& src);
		void setReplyBuffer(const Message& msg);
		void addToCmdBuffer(const string& src);
		void addToReplyBuffer(const string& src);
		void addToReplyBuffer(const Message& msg);

		void addToMyChannelList(Channel* channel);
		void deleteFromMyChannelList(Channel* channel);
		void clearMyChannelList(void);
};

#endif
