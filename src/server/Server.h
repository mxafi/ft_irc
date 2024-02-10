#ifndef SERVER_H
#define SERVER_H

#include <string>

namespace irc {

class Server {
 private:
  int server_socket_fd_;
  int port_;
  std::string password_;

 public:
  ~Server(){};
  Server(int port, std::string password);
  void start();
  void loop();
};

}  // namespace irc

#endif
