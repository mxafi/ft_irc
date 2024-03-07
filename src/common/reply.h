#ifndef REPLY_H
#define REPLY_H

// Meta definitions
#define RPL_META_MESSAGE(servername, numeric, message) (std::string(":") + servername + " " + numeric + " " + message + "\r\n")
#define ERR_MESSAGE(message) (std::string("ERROR :") + message + "\r\n")
#define COM_MESSAGE(nick, user, host, command, params) (std::string(":") + nick + "!" + user + "@" + host + " " + command + " " + params + "\r\n")

// Numeric replies in order
#define RPL_WELCOME_001(servername, nick, user, host) (RPL_META_MESSAGE(servername, "001", ":Welcome to the Internet Relay Network " + nick + "!" + user + "@" + host))
#define RPL_YOURHOST_002(servername, version) (RPL_META_MESSAGE(servername, "002", ":Your host is " + servername + ", running version " + version))
#define RPL_CREATED_003(servername, date) (RPL_META_MESSAGE(servername, "003", ":This server was created " + date))
#define RPL_MYINFO_004(servername, version, user_modes, channel_modes) (RPL_META_MESSAGE(servername, "004", servername + " " + version + " " + user_modes + " " + channel_modes))

#define RPL_NOTOPIC_331(servername, channel) (RPL_META_MESSAGE(servername, "331", channel + " :No topic is set"))
#define RPL_TOPIC_332(servername, channel, topic) (RPL_META_MESSAGE(servername, "332", channel + " :" + topic))
#define RPL_NAMREPLY_353(servername, client, symbol, channel, namelist) (RPL_META_MESSAGE(servername, "353", client + " " + symbol + " " + channel + " :" + namelist))

#define RPL_ERR_NOSUCHCHANNEL_403(servername, channel) (RPL_META_MESSAGE(servername, "403", channel + " :No such channel"))
#define RPL_ERR_TOOMANYCHANNELS_405(servername, channel) (RPL_META_MESSAGE(servername, "405", channel + " :You have joined too many channels"))
#define RPL_ERR_UNKNOWNCOMMAND_421(servername, command) (RPL_META_MESSAGE(servername, "421", command + " :Unknown command"))
#define RPL_ERR_NONICKNAMEGIVEN_431(servername) (RPL_META_MESSAGE(servername, "431", ":No nickname given"))
#define RPL_ERR_NOTONCHANNEL_442(servername, channel) (RPL_META_MESSAGE(servername, "442", channel + " :You're not on that channel"))
#define RPL_ERR_NOTREGISTERED_451(servername) (RPL_META_MESSAGE(servername, "451", ":You have not registered"))
#define RPL_ERR_NEEDMOREPARAMS_461(servername, command) (RPL_META_MESSAGE(servername, "461", command + " :Not enough parameters"))
#define RPL_ERR_ALREADYREGISTRED_462(servername) (RPL_META_MESSAGE(servername, "462", ":Unauthorized command (already registered)"))
#define RPL_ERR_CHANNELISFULL_471(servername, channel) (RPL_META_MESSAGE(servername, "471", channel + " :Cannot join channel (+l)"))
#define RPL_ERR_INVITEONLYCHAN_473(servername, channel) (RPL_META_MESSAGE(servername, "473", channel + " :Cannot join channel (+i)"))
#define RPL_ERR_BADCHANNELKEY_475(servername, channel) (RPL_META_MESSAGE(servername, "475", channel + " :Cannot join channel (+k)"))
#define RPL_ERR_ERRONEUSNICKNAME_432(servername, nick) (RPL_META_MESSAGE(servername, "432", nick + " :Erroneous nickname"))
#define RPL_ERR_NICKNAMEINUSE_433(servername, nick) (RPL_META_MESSAGE(servername, "433", nick + " :Nickname is already in use"))

#endif
