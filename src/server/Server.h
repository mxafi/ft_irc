#ifndef SERVER_H
#define SERVER_H

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

#include "../common/log.h"
#include "../common/magicNumber.h"

#include "../client/Client.h"

extern bool isServerRunning_g;
extern std::string serverHostname_g;

namespace irc {

class Server {
 private:
  int setServerHostname_();
  char* port_;
  std::string password_;
  int server_socket_fd_;
  int server_socket_domain_ = AF_INET;
  int server_socket_type_ = SOCK_STREAM;
  int server_socket_protocol_ = DEFAULT_SOCKET_PROTOCOL;
  struct addrinfo hints_;
  struct addrinfo* srvinfo_;
  std::vector<Client> clients_;

 public:
  ~Server();
  Server(char* port, std::string password);
  int start();
  void loop();
  char* getPort();
  std::string getPassword();
  int getServerSocketFd();
  int getServerSocketDomain();
  int getServerSocketType();
  int getServerSocketProtocol();
  struct addrinfo& getServerInfo();
};

}  // namespace irc

#endif
