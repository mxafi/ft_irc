#include "../catch2/catch_amalgamated.hpp"

#include <string>
#include <cerrno>
#include "../../src/server/Server.h"
#include "../../src/common/magicNumber.h"

TEST_CASE("server is started properly", "[server]") {
  char port[] = "6667";
  std::string password = "horse";
  irc::Server server(port, password);

  REQUIRE(isServerRunning_g == false);
  int startValue = server.start();
  REQUIRE(startValue == 0);  // 0 is the value of SUCCESS
  REQUIRE(isServerRunning_g == true);
  REQUIRE(serverHostname_g.empty() == false);
  REQUIRE(server.getServerSocketFd() > 0);  // socket() returns a positive int
  REQUIRE(server.getServerSocketDomain() == AF_INET);
  REQUIRE(server.getServerSocketType() == SOCK_STREAM);
  REQUIRE(server.getServerSocketProtocol() == DEFAULT_SOCKET_PROTOCOL);
  REQUIRE(server.getServerInfo().ai_family == AF_INET);
  REQUIRE(server.getServerInfo().ai_socktype == SOCK_STREAM);
  REQUIRE(server.getServerInfo().ai_protocol == TCP_PROTOCOL);
  REQUIRE(server.getPassword() == password);
  REQUIRE(server.getPort() == port);
  REQUIRE(errno == 0);
}
