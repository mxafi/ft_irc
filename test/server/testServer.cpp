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
  REQUIRE(errno == errno_before);
}
