#ifndef REPLY_H
#define REPLY_H

// Meta definitions
#define RPL_META_MESSAGE(servername, numeric, message) (std::string(":") + servername + " " + numeric + " " + message + "\r\n")
#define ERR_MESSAGE(message) (std::string("ERROR :") + message + "\r\n")

// Numeric replies in order
#define RPL_WELCOME_001(servername, nick, user, host) (RPL_META_MESSAGE(servername, "001", "Welcome to the Internet Relay Network " + nick + "!" + user + "@" + host))
#define RPL_YOURHOST_002(servername, version) (RPL_META_MESSAGE(servername, "002", "Your host is " + servername + ", running version " + version))
#define RPL_CREATED_003(servername, date) (RPL_META_MESSAGE(servername, "003", "This server was created " + date))
#define RPL_MYINFO_004(servername, version, user_modes, channel_modes) (RPL_META_MESSAGE(servername, "004", servername + " " + version + " " + user_modes + " " + channel_modes))

#define RPL_ERR_UNKNOWNCOMMAND_421(servername, command) (RPL_META_MESSAGE(servername, "421", command + " :Unknown command"))
#define RPL_ERR_NONICKNAMEGIVEN_431(servername) (RPL_META_MESSAGE(servername, "431", ":No nickname given"))
#define RPL_ERR_NOTREGISTERED_451(servername) (RPL_META_MESSAGE(servername, "451", ":You have not registered"))
#define RPL_ERR_NEEDMOREPARAMS_461(servername, command) (RPL_META_MESSAGE(servername, "461", command + " :Not enough parameters"))
#define RPL_ERR_ALREADYREGISTRED_462(servername) (RPL_META_MESSAGE(servername, "462", ":Unauthorized command (already registered)"))
#define RPL_ERR_ERRONEUSNICKNAME_432(servername, nick) (RPL_META_MESSAGE(servername, "432", nick + " :Erroneous nickname"))
#define RPL_ERR_ERR_NICKNAMEINUSE_433(servername, nick) (RPL_META_MESSAGE(servername, "433", nick + " :Nickname is already in use"))

#endif
