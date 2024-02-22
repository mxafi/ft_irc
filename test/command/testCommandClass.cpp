#include "../catch2/catch_amalgamated.hpp"

#include "../../src/command/Command.h"


using namespace irc;

TEST_CASE("Command class works as expected", "[command]") {
    int errno_before = errno;
    REQUIRE(errno == errno_before);
  SECTION("Valid message with prefix and parameters") {
    std::string message =
        ":nick!~user@host.com PRIVMSG #channel :Hello, World!";
    Message msg(message);
    REQUIRE(msg.getPrefix() == ":nick!~user@host.com");
    REQUIRE(msg.getCommand() == "PRIVMSG");
    REQUIRE(msg.getParameters().size() == 2);
    REQUIRE(msg.getParameters()[0] == "#channel");
    REQUIRE(msg.getParameters()[1] == ":Hello, World!");
    REQUIRE(msg.getNumeric() == 0);
    REQUIRE(errno == errno_before);
  }
}