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
#include <ctime>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

#include "../common/log.h"
#include "../common/magicNumber.h"
#include "../common/os.h"

#include "../client/Client.h"
#include "../message/Message.h"
#include "../command/Command.h"

extern bool isServerRunning_g;
extern std::string serverHostname_g;

namespace irc {

/**
 * @class Server
 * @brief Represents an IRC server that handles client connections and messages.
 * 
 * The Server class provides functionality for starting an IRC server, accepting client connections,
 * sending and receiving messages, and handling client disconnections. It also stores information
 * about the server's hostname, port, password, and other configuration details.
 */
class Server {
 private:
  int setServerHostname_();
  int acceptClient_(std::vector<pollfd>& pollfds);
  int disconnectClient_(std::vector<pollfd>& poll_fds,
                        std::vector<pollfd>::iterator& it);
  long long sendFromBuffer_(Client& client);
  long long recvToBuffer_(Client& client);
  int extractMessageString_(std::string& message, Client& client);
  void handleMalformedMessage_(Client& client, Message& message);
  char* port_;
  std::string password_;
  int server_socket_fd_;
  int server_socket_domain_ = AF_INET;
  int server_socket_type_ = SOCK_STREAM;
  int server_socket_protocol_ = DEFAULT_SOCKET_PROTOCOL;
  struct addrinfo hints_;
  struct addrinfo* srvinfo_;
  std::map<int, Client> clients_;
  time_t start_time_;

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
  std::string getStartTimeString();
};

}  // namespace irc

#endif
