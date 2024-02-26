#ifndef OS_H
#define OS_H

#ifdef __APPLE__
#define ON_MACOS true
#define MSG_NOSIGNAL 0
#else
#define ON_MACOS false
#define SO_NOSIGPIPE 0
#endif

#endif
