#ifndef SERVER_H
#define SERVER_H

#include <fcntl.h>
#include <netdb.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

#include "../common/log.h"
#include "../common/magicNumber.h"

extern bool isServerRunning_g;

namespace irc {

class Server {
 private:
  char* port_;
  std::string password_;
  int server_socket_fd_;
  int server_socket_domain_ = AF_INET;
  int server_socket_type_ = SOCK_STREAM;
  int server_socket_protocol_ = DEFAULT_SOCKET_PROTOCOL;
  struct addrinfo hints_;
  struct addrinfo* srvinfo_;

 public:
  ~Server();
  Server(char* port, std::string password);
  int start();
  void loop();
};

}  // namespace irc

#endif
