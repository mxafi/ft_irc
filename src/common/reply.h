#ifndef REPLY_H
#define REPLY_H

// Meta definitions
#define RPL_META_MESSAGE(servername, numeric, message) (std::string(":") + servername + " " + numeric + " " + message + "\r\n")

// Numeric replies in order
#define RPL_WELCOME_001(servername, nick, user, host) (RPL_META_MESSAGE(servername, "001", "Welcome to the Internet Relay Network " + nick + "!" + user "@" + host))
#define RPL_YOURHOST_002(servername, version) (RPL_META_MESSAGE(servername, "002", "Your host is " + servername + ", running version " + version))
#define RPL_CREATED_003(servername, date) (RPL_META_MESSAGE(servername, "003", "This server was created " + date))
#define RPL_MYINFO_004(servername, version, user_modes, channel_modes) (RPL_META_MESSAGE(servername, "004", servername + " " + version + " " + user_modes + " " + channel_modes))

#endif
