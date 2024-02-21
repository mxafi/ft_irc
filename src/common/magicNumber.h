#ifndef MAGICNUMBER_H
#define MAGICNUMBER_H

#define SUCCESS 0
#define FAILURE 1
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
#define GETHOSTNAME_FAILURE -1
#define HOSTNAME_MAX_LENGTH 64
#define SOCKET_FAILURE -1
#define FCNTL_FAILURE -1
#define BIND_FAILURE -1
#define LISTEN_FAILURE -1
#define DEFAULT_SOCKET_PROTOCOL 0
#define TCP_PROTOCOL 6
#define POLL_FAILURE -1
#define MESSAGE_MAX_AMOUNT_PARAMETERS 15
#define MAX_MSG_LENGTH 512
#define NICK_MAX_LENGTH_RFC2812 9

// Numeric reply names and numbers
#define ERR_INPUTTOOLONG 417

// Custom numeric replies
// According to RFC2812 Error codes > 502 are free, and on IRCv3 > 525 and < 670
#define ERR_CUSTOM_ILLEGALNUL 530
#define ERR_CUSTOM_TOOMANYPARAMS 531

#endif
