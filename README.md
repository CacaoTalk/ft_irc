# ft_irc
Implement Internet Relay Chat Server 📢 

## 🎯 TO DO
- Implement a server that manages **multiple clients** as a **single thread** using **I/O multiplexing**
- The server **get messages** from clients and **send these messages** to channel(s)/client(s) who is currently connected to this server
- Should be accessed and used with several **reference IRC clients**(Follow the IRC RFC documentation) [RFC 1492](https://www.rfc-editor.org/rfc/rfc1459) / [RFC 2812](https://www.rfc-editor.org/rfc/rfc2812)

## ✔️ Getting Started
### How to execute server
```bash
make
./ircserv <port> <password>
```
<img width="720" alt="image" src="https://user-images.githubusercontent.com/60038526/218292154-d8e119f1-ace8-4d92-93de-ae9720448ba8.png">
<img width="720" alt="image" src="https://user-images.githubusercontent.com/60038526/218292195-06beed8f-f0f4-4000-9d92-cc504c1739d8.png">


### To change server settings
- You can change the server settings in the **CommonValue.hpp** file.
- After changing the settings, enter **"make re"** to compile a new server.
<img width="720" alt="image" src="https://user-images.githubusercontent.com/60038526/218293024-040b6834-8432-42e9-af2a-a99dc2b72fe1.png">

- Customizable settings

|SETTING|DEFAULT VALUE|
|-|-|
|MAX_NICKNAME_LEN|9|
|MAX_CHANNELNAME_LEN|31|
|MAX_USER_NUM|30|
|MAX_CHANNEL_NUM|30|
|SERVER_HOSTNAME|"cacaotalk.42seoul.kr"|
|DEFAULT_PART_MESSAGE|" leaved channel."|
|NEW_OPERATOR_MESSAGE|" is new channel operator."|

### Connect to server with client
- You can connect to this server using reference IRC clients(such as [irssi](https://irssi.org/))
<img width="710" alt="image" src="https://user-images.githubusercontent.com/60038526/218292258-1e38f9e9-962d-475c-931e-9bf6ec668ff4.png">
<img width="710" alt="image" src="https://user-images.githubusercontent.com/60038526/218292294-e320a3b6-826c-40cc-8475-558bc7b5c006.png">


## ✨ Feature
### General commands
|COMMAND|DESCRIPTION|
|-|-|
|PASS|used to set a **'connection password'**|
|NICK|used to give **user a nickname** or change the existing one.|
|USER|used at the beginning of connection to **specify the username, hostname and realname of a new user**|
|JOIN|used by a user to request to **start listening** to the specific **channel**.|
|PRIVMSG|used to **send private messages** between users, as well as to send messages to channels.|
|NOTICE|used similarly to PRIVMSG. The difference is automatic **replies never be sent in response** to a NOTICE message.|
|KICK|used to request the **forced removal of a user** from a channel. (for channel operator only)|
|PART|used to **leave from the channel** user belong to.|
|QUIT|A client session **is terminated** with a quit message.|

---

### [DCC](https://modern.ircdocs.horse/dcc.html)(Direct Client-to-Client) support
- With this function, you can chat or transfer files between clients **without going through the server**
- **`Warning: DCC has no encryption`**
- DCC Query Syntax
```bash
The initial CTCP `DCC` query message has this format:
DCC <type> <argument> <host> <port>
```
<img width="710" alt="image" src="https://user-images.githubusercontent.com/60038526/218292719-a57f4396-fc4a-47d3-b5e1-4ef3d27a2827.png">

---

### Bot commands
- It provides a function to **randomly select** one of the menus added by channel members.
- Duplicate registration is possible, so you can weight it as needed.

|COMMAND|DESCRIPTION|
|-|-|
|!addmenu|Add to the menu list following param(s). You can use space to add multiple things at the same time.|
|!deletemenu|Delete from the menu list following param(s). You can use space to delete multiple things at the same time.|
|!showmenu|Print a list of menus currently in the list.|
|!pickmenu|Recommend one of the menus in the current list randomly.|

<img width="710" alt="image" src="https://user-images.githubusercontent.com/60038526/218292832-fda47a01-a8ce-4614-8d57-47eb1619fa93.png">
