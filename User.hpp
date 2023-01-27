#pragma once

#ifndef USER_HPP
# define USER_HPP

# include <string>
using namespace std;

class User {
	private:
		int _fd;
		string _password;
		string _nickname; // unique
		string _username;
		bool _auth;
		string _cmdBuffer;
		string _replyBuffer;

		User(void);
		User(const User& user);
		User& operator=(const User& user);

	public:
        User(int fd);
		~User();
		
		int getFd(void);
		string getPassword(void) const;
		string getNickname(void) const;
		string getUsername(void) const;
		bool getAuth(void) const;
		string getCmdBuffer(void);
		const string getCmdBuffer(void) const;
		string getReplyBuffer(void);
		const string getReplyBuffer(void) const;

		void setPassword(const string& pwd);
		void setNickname(const string& nickname);
		void setUsername(const string& username);
		void setAuth();
		void setCmdBuffer(const string& src);
		void setReplyBuffer(const string& src);
		void addToCmdBuffer(const string& src); // 채팅 받아오기
		void addToReplyBuffer(const string& src); // write할 버퍼 추가
};

#endif