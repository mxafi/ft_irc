#ifndef MAGICNUMBER_H
#define MAGICNUMBER_H

#define IRC_SERVER_VERSION "ft_irc-v1.0"
#define SUPPORTED_USER_MODES "io"
#define SUPPORTED_CHANNEL_MODES "oitkl"
#define SUPPORTED_CHANNEL_MODES_PLUS_WITH_PARAM "okl"
#define SUPPORTED_CHANNEL_MODES_MINUS_WITH_PARAM "ok"
#define CHANNEL_PREFIXES "#&"
#define CHANNEL_SYMBOL_PUBLIC "="
#define CHANNEL_OPERATOR_SYMBOL "@"
#define CHANNEL_USER_LIMIT_DISABLED -1
#define MAX_CHANNELNAME_LENGTH 50
#define MIN_CHANNELNAME_LENGTH 2
#define MAX_JOIN_CHANNELS 20
#define CHANNEL_PART_FAILURE -1
#define SUCCESS 0
#define FAILURE 1
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
#define TRUE 1
#define FALSE 0
#define GETHOSTNAME_FAILURE -1
#define HOSTNAME_MAX_LENGTH 64
#define SOCKET_FAILURE -1
#define FCNTL_FAILURE -1
#define SETSOCKOPT_FAILURE -1
#define BIND_FAILURE -1
#define LISTEN_FAILURE -1
#define ACCEPT_FAILURE -1
#define SEND_FAILURE -1
#define RECV_FAILURE -1
#define DEFAULT_SOCKET_PROTOCOL 0
#define TCP_PROTOCOL 6
#define POLL_FAILURE -1
#define MESSAGE_MAX_AMOUNT_PARAMETERS 15
#define MAX_MSG_LENGTH 512
#define NICK_MAX_LENGTH_RFC2812 9
#define SERVER_RECV_BUFFER_SIZE 1024
#define RECV_ORDERLY_SHUTDOWN 0

// Numeric reply names and numbers
#define ERR_INPUTTOOLONG 417
#define RPL_WELCOME 001
#define RPL_YOURHOST 002
#define RPL_CREATED 003
#define RPL_MYINFO 004

// Custom numeric replies
// According to RFC2812 Error codes > 502 are free, and on IRCv3 > 525 and < 670
#define ERR_CUSTOM_ILLEGALNUL 530
#define ERR_CUSTOM_TOOMANYPARAMS 531
#define ERR_NICKNAMEINUSE 433
#define ERR_ERRONEUSNICKNAME 432
#define ERR_NONICKNAMEGIVEN 431

#endif
