#include "../catch2/catch_amalgamated.hpp"

#include <string>
#include <cerrno>
#include "../../src/server/Server.h"

TEST_CASE("server object is initialised properly", "[server]") {
  int errno_before = errno;
  char port[] = "6667";
  std::string password = "horse";
  irc::Server server(port, password);

  int startValue = server.start();
  REQUIRE(startValue == 0);  // 0 is the value of SUCCESS
  REQUIRE(serverHostname_g.empty() == false);
  REQUIRE(server.getServerSocketFd() > 0);  // socket() returns a positive int
  REQUIRE(server.getServerSocketDomain() == AF_INET);
  REQUIRE(server.getServerSocketType() == SOCK_STREAM);
  REQUIRE(server.getServerSocketProtocol() == DEFAULT_SOCKET_PROTOCOL);
  REQUIRE(server.getServerInfo().ai_family == AF_INET);
  REQUIRE(server.getServerInfo().ai_socktype == SOCK_STREAM);
  REQUIRE(server.getServerInfo().ai_protocol == DEFAULT_SOCKET_PROTOCOL);
  REQUIRE(errno == errno_before);
}
