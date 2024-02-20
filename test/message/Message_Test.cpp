#include "../../src/message/Message.h"
#include "../catch2/catch_amalgamated.hpp"

using namespace irc;

TEST_CASE("Message deserialization", "[message]") {
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
  }

  SECTION("Valid message without prefix") {
    std::string message = "PRIVMSG #channel :Hello, World!";
    Message msg(message);

    REQUIRE(msg.getPrefix() == "");
    REQUIRE(msg.getCommand() == "PRIVMSG");
    REQUIRE(msg.getParameters().size() == 2);
    REQUIRE(msg.getParameters()[0] == "#channel");
    REQUIRE(msg.getParameters()[1] == ":Hello, World!");
    REQUIRE(msg.getNumeric() == 0);
  }

  SECTION("Invalid message with too many parameters") {
    std::string message =
        ":nick!~user@host.com PRIVMSG #channel param2 param3 param4 param5 "
        "param6 param7 param8 param9 param10 param11 param12 param13 param14 "
        "param15 param16";
    Message msg(message);
    REQUIRE(msg.getNumeric() == ERR_CUSTOM_TOOMANYPARAMS);
  }

  SECTION("Input message too long") {
    std::string message =
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed sed "
        "tristique eros. Pellentesque fermentum massa et felis mattis finibus. "
        "Cras vulputate, quam et finibus efficitur, libero ante porttitor "
        "sapien, at congue dolor nisi in orci. Suspendisse ultrices porta "
        "erat, ut suscipit lectus tempus ac. Nullam commodo velit eu iaculis "
        "aliquet. Vestibulum ac mi id quam lacinia condimentum. In laoreet leo "
        "in enim rhoncus pretium. Nunc feugiat ullamcorper felis eget euismod. "
        "Vivamus consectetur dui eget dapibus bibendum. Fusce id libero sed "
        "nunc donec.";
    Message msg(message);
    REQUIRE(msg.getNumeric() == ERR_CUSTOM_TOOMANYPARAMS);
  }

}
