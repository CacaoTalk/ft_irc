#ifndef REPLY_HPP
# define REPLY_HPP

// NUMERIC REPLIES
# define RPL_WELCOME "001"

# define RPL_NAMREPLY "353"
# define RPL_ENDOFNAMES "366"
# define RPL_ENDOFNAMES_MSG ":End of /NAMES list."

# define ERR_UNKNOWNERROR "400"
# define ERR_NOSUCHNICK_MSG ":No such nick/channel"
# define ERR_NOSUCHNICK "401"
# define ERR_NOSUCHSERVER "402"
# define ERR_NOSUCHCHANNEL "403"
# define ERR_NOSUCHCHANNEL_MSG ":No such channel"
# define ERR_CANNOTSENDTOCHAN "404"
# define ERR_TOOMANYCHANNELS "405"
# define ERR_NOORIGIN "409"
# define ERR_NOORIGIN_MSG ":No origin specified"
# define ERR_NORECIPIENT "411"
# define ERR_NORECIPIENT_MSG ":No recipient given"
# define ERR_NOTEXTTOSEND "412"
# define ERR_NOTEXTTOSEND_MSG ":No text to send"

# define ERR_UNKNOWNCOMMAND "421"
# define ERR_UNKNOWNCOMMAND_MSG ":Unknown command"

# define ERR_NONICKNAMEGIVEN "431"
# define ERR_NONICKNAMEGIVEN_MSG ":No nickname given"
# define ERR_ERRONEUSNICKNAME "432"
# define ERR_ERRONEUSNICKNAME_MSG ":Erroneous nickname"
# define ERR_NICKNAMEINUSE "433"
# define ERR_NICKNAMEINUSE_MSG ":Nickname is already in use"
# define ERR_UNAVAILRESOURCE "437"
# define ERR_UNAVAILRESOURCE_MSG ":Channel is temporarily unavailable"

# define ERR_USERNOTINCHANNEL "441"
# define ERR_USERNOTINCHANNEL_MSG ":They aren't on that channel"
# define ERR_NOTONCHANNEL "442"
# define ERR_NOTONCHANNEL_MSG ":You're not on that channel"
# define ERR_USERONCHANNEL "443"

# define ERR_NEEDMOREPARAMS "461"
# define ERR_NEEDMOREPARAMS_MSG ":Not enough parameters"
# define ERR_ALREADYREGISTERED "462"
# define ERR_ALREADYREGISTERED_MSG ":You may not reregister"
# define ERR_PASSWDMISMATCH "464"
# define ERR_PASSWDMISMATCH_MSG ":Password incorrect"

# define ERR_CHANNELISFULL "471"
# define ERR_ERRONEUSCHANNELNAME "479"
# define ERR_ERRONEUSCHANNELNAME_MSG ":Channel name contains illegal characters"
# define ERR_CHANOPRIVSNEEDED "482"
# define ERR_CHANOPRIVSNEEDED_MSG ":You're not channel operator"

#endif
