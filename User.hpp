#pragma once

#ifndef USER_HPP
# define USER_HPP

# include <string>
using namespace std;

class User {
	private:
		int _fd;
		string _nickname; // unique
		string _username;
		string _cmdBuffer;
		string _replyBuffer;

		User(void);
		User(const User& user);
		User& operator=(const User& user);

	public:
        User(int fd);
		~User();
		
		string getNickname(void) const;
		string getCmdBuffer(void);
		string getReplyBuffer(void);
		void setCmdBuffer(const string& src);
		void setReplyBuffer(const string& src);
		void addToCmdBuffer(const string& src); // 채팅 받아오기
		void addToReplyBuffer(const string& src); // write할 버퍼 추가
};

#endif