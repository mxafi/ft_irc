#include "../catch2/catch_amalgamated.hpp"

#include <cerrno>
#include <map>
#include "../../src/client/Client.h"
#include "../../src/command/Command.h"
#include "../../src/message/Message.h"

extern std::string serverHostname_g;
using namespace irc;

// TEST_CASE("Command class works as expected", "[command]") {
//   int errno_before = errno;
//   REQUIRE(errno == errno_before);
//   struct sockaddr sockaddr;
//   Client diego(1, sockaddr);
//   Client pedro(2, sockaddr);
//   Client jesus(3, sockaddr);
//   diego.setUserName("djames");
//   diego.setPassword("horse");
//   std::string password = "horse";
//   time_t serverStartTime = time(NULL);
//   std::map<int, Client> myClients = {{1, diego}, {2, pedro}, {3, jesus}};

//   SECTION("Valid message command nick") {
//     std::string message = "NICK hola";
//     diego.setNickname("hola");
//     std::string response = ":" + diego.getOldNickname() + "!" +
//                            diego.getUserName() + "@" + serverHostname_g +
//                            " NICK :" + diego.getNickname() + "\n";
//     Message msg(message);
//     Command nick(msg, diego, myClients, password, serverStartTime);
//     REQUIRE(diego.getSendBuffer() == response);
//   }
//   SECTION("if it doesnt have a nick") {
//     std::string message = "NICK";
//     std::string response = ":No nickname given\n";
//     Message msg(message);
//     Command nick(msg, diego, myClients, password, serverStartTime);
//     REQUIRE(diego.getSendBuffer() == response);
//     REQUIRE(errno == errno_before);
//   }
//   SECTION("if there is the same nickname") {
//     std::string message = "NICK papa";
//     std::string response = ":No nickname given\n";
//     Message msg(message);
//     Command nick(msg, diego, myClients, password, serverStartTime);
//     REQUIRE(diego.getSendBuffer() == response);
//     REQUIRE(errno == errno_before);
//   }

// }
TEST_CASE("Command PRIVMSG action", "[command][privmsg]") {
  int errno_before = errno;
  REQUIRE(errno == errno_before);
  std::string serverHostname = "test.server.com";
  time_t serverStartTime = time(NULL);
  std::string password = "password";
  struct sockaddr sockaddr;

  Client client(1, sockaddr);
  client.setPassword("password");
  client.setUserName("userName");
  client.setNickname("nickname");

  std::map<int, Client> myClients = {{1, client}};

  SECTION("PRIVMSG with no parameters") {
    std::string response = ": 411 :No recipient given (privmsg)\r\n";
    std::string msgWithoutParameters = "PRIVMSG";
    Message msg(msgWithoutParameters);
    Command cmd(msg, client, myClients, password, serverStartTime);
    client.clearSendBuffer();
    cmd.actionPrivmsg(client);
    REQUIRE(client.getSendBuffer() == response );
  }
}
