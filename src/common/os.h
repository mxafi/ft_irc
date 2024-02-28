#ifndef OS_H
#define OS_H

#ifdef __APPLE__
#define ON_MACOS true
#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif  // MSG_NOSIGNAL
#else   // __APPLE__
#define ON_MACOS false
#ifndef SO_NOSIGPIPE
#define SO_NOSIGPIPE 0
#endif  // SO_NOSIGPIPE
#endif  // __APPLE__

#endif  // OS_H
