#include "../../src/client/Client.h"
#include "../catch2/catch_amalgamated.hpp"

#include <map>
#include "../../src/command/Command.h"
#include "../../src/message/Message.h"

extern std::string serverHostname_g;
using namespace irc;

TEST_CASE("Command class works as expected", "[command]") {
  int errno_before = errno;
  REQUIRE(errno == errno_before);
  struct sockaddr sockaddr;
  Client diego(1, sockaddr);
  Client pedro(2, sockaddr);
  Client jesus(3, sockaddr);
  diego.setUserName("djames");
  std::map<int, Client> myClients = {{1, diego}, {2, pedro}, {3, jesus}};

  SECTION("Valid message command nick") {
    std::string message = "nick hola";
    diego.setNickname("hola");
    std::string response = ":" + diego.getOldNickname() + "!" +
                           diego.getUserName() + "@" + serverHostname_g +
                           " NICK :" + diego.getNickname() + "\n";
    Message msg(message);
    Command nick(msg, diego, myClients);
    REQUIRE(diego.getSendBuffer() == response);
  }
  SECTION("if it doesnt have a nick") {
    std::string message = "nick";
    std::string response = ":No nickname given\n";
    Message msg(message);
    Command nick(msg, diego, myClients);
    REQUIRE(diego.getSendBuffer() == response);
    //REQUIRE(errno == errno_before);
  }
  SECTION("if there is the same nickname") {
    std::string message = "nick papa";
    std::string response = ":No nickname given\n";
    Message msg(message);
    Command nick(msg, diego, myClients);
    REQUIRE(diego.getSendBuffer() == response);
    //REQUIRE(errno == errno_before);
  }

  // more test for the comands
}