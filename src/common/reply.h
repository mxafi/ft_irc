#ifndef REPLY_H
#define REPLY_H

// Meta definitions
#define RPL_META_MESSAGE(servername, numeric, message) (std::string(":") + servername + " " + numeric + " " + message + "\r\n")
#define ERR_MESSAGE(message) (std::string("ERROR :") + message + "\r\n")
#define COM_MESSAGE(nick, user, host, command, params) \
    (std::string(":") + nick + "!" + user + "@" + host + " " + command + " " + params + "\r\n")
#define RPL_MESSAGE(message) (std::string(message) + "\r\n")

//User format
#define FORMAT_NICK_USER_HOST(nickname, username, hostname) (std::string(":") + nickname + "!" + username + "@" + hostname)
#define PRIVMSG_FORMAT(formattedSender, target, text) (RPL_MESSAGE(std::string(formattedSender) + " PRIVMSG " + target + " :" + text))
#define INVITE(nickname, username, hostname, invitee, channel) (RPL_MESSAGE(std::string(":") + nickname + "!" + username + "@" + hostname + " INVITE " + invitee + " :" + channel))

// Numeric replies in order
#define RPL_WELCOME_001(servername, nick, user, host) \
    (RPL_META_MESSAGE(servername, "001", nick + " :Welcome to the Internet Relay Network " + nick + "!" + user + "@" + host))
#define RPL_YOURHOST_002(servername, nick, version) \
    (RPL_META_MESSAGE(servername, "002", nick + " :Your host is " + servername + ", running version " + version))
#define RPL_CREATED_003(servername, nick, date) (RPL_META_MESSAGE(servername, "003", nick + " :This server was created " + date))
#define RPL_MYINFO_004(servername, nick, version, user_modes, channel_modes) \
    (RPL_META_MESSAGE(servername, "004", nick + " " + servername + " " + version + " " + user_modes + " " + channel_modes))

#define RPL_CHANNELMODEIS_324(servername, client, channel, enabled_modes) (RPL_META_MESSAGE(servername, "324", client + " " + channel + " +" + enabled_modes))
#define RPL_NOTOPIC_331(servername, client, channel) (RPL_META_MESSAGE(servername, "331", client + " " + channel + " :No topic is set"))
#define RPL_TOPIC_332(servername, client, channel, topic) (RPL_META_MESSAGE(servername, "332", client + " " + channel + " :" + topic))
#define RPL_INVITING_341(servername, inviter, invitee, channel) (RPL_META_MESSAGE(servername, "341", inviter + " " + invitee + " " + channel))
#define RPL_NAMREPLY_353(servername, client, symbol, channel, namelist) \
    (RPL_META_MESSAGE(servername, "353", client + " " + symbol + " " + channel + " :" + namelist))

#define RPL_ERR_NOSUCHNICK_401(servername, nick_or_channel) \
    (RPL_META_MESSAGE(servername, "401", nick_or_channel + " :No such nick/channel"))
#define RPL_ERR_NOSUCHCHANNEL_403(servername, channelName) (RPL_META_MESSAGE(servername, "403", channelName + " :No such channel"))
#define RPL_ERR_CANNOTSENDTOCHAN_404(servername, channelName) (RPL_META_MESSAGE(servername, "404", channelName + ":Cannot send to channel"))
#define RPL_ERR_TOOMANYCHANNELS_405(servername, channel) \
    (RPL_META_MESSAGE(servername, "405", channel + " :You have joined too many channels"))
#define RPL_ERR_TOOMANYTARGETS_407(servername, target, errorcode, abortmessage) \
    (RPL_META_MESSAGE(servername, "407", target + " :" + errorcode + " recipients. " + abortmessage))
#define RPL_ERR_NORECIPIENT_411(servername, command) (RPL_META_MESSAGE(servername, "411", +":No recipient given (" + command + ")"))
#define RPL_ERR_NOTEXTTOSEND_412(servername) (RPL_META_MESSAGE(servername, "412", +":No text to send"))

#define RPL_ERR_UNKNOWNCOMMAND_421(servername, command) (RPL_META_MESSAGE(servername, "421", command + " :Unknown command"))
#define RPL_ERR_NONICKNAMEGIVEN_431(servername) (RPL_META_MESSAGE(servername, "431", ":No nickname given"))
#define RPL_ERR_ERRONEUSNICKNAME_432(servername, nick) (RPL_META_MESSAGE(servername, "432", nick + " :Erroneous nickname"))
#define RPL_ERR_NICKNAMEINUSE_433(servername, client, nick) (RPL_META_MESSAGE(servername, "433", client + " " + nick + " :Nickname is already in use"))
#define RPL_ERR_USERNOTINCHANNEL_441(servername, nick, channel) (RPL_META_MESSAGE(servername, "441", nick + " " + channel + " :They aren't on that channel"))
#define RPL_ERR_NOTONCHANNEL_442(servername, client, channel) (RPL_META_MESSAGE(servername, "442", client + " " + channel + " :You're not on that channel"))
#define RPL_ERR_USERONCHANNEL_443(servername, client, nick, channel) (RPL_META_MESSAGE(servername, "443", client + " " + nick + " " + channel + " :is already on channel"))
#define RPL_ERR_NOTREGISTERED_451(servername) (RPL_META_MESSAGE(servername, "451", ":You have not registered"))
#define RPL_ERR_NEEDMOREPARAMS_461(servername, client, command) (RPL_META_MESSAGE(servername, "461", client + " " + command + " :Not enough parameters"))
#define RPL_ERR_ALREADYREGISTRED_462(servername, client) (RPL_META_MESSAGE(servername, "462", client + " :You may not reregister"))
#define RPL_ERR_KEYSET_467(servername, channel) (RPL_META_MESSAGE(servername, "467", channel + " :Channel key already set"))
#define RPL_ERR_CHANNELISFULL_471(servername, client, channel) (RPL_META_MESSAGE(servername, "471", client + " " + channel + " :Cannot join channel (+l)"))
#define RPL_ERR_UNKNOWNMODE_472(servername, client, mode, channel) (RPL_META_MESSAGE(servername, "472", client + " " + mode + " :is unknown mode char to me for " + channel))
#define RPL_ERR_INVITEONLYCHAN_473(servername, client, channel) (RPL_META_MESSAGE(servername, "473", client + " " + channel + " :Cannot join channel (+i)"))
#define RPL_ERR_BADCHANNELKEY_475(servername, client, channel) (RPL_META_MESSAGE(servername, "475", client + " " + channel + " :Cannot join channel (+k)"))
#define RPL_ERR_CHANOPRIVSNEEDED_482(servername, client, channel) (RPL_META_MESSAGE(servername, "482", client + " " + channel + " :You're not channel operator"))

#define RPL_ERR_UMODEUNKNOWNFLAG_501(servername, client) (RPL_META_MESSAGE(servername, "501", client + " :Unknown MODE flag"))

#endif
