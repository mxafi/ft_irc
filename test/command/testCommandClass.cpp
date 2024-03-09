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
  std::string serverHostname = serverHostname_g;
  time_t serverStartTime = time(NULL);
  std::string password = "password";
  struct sockaddr sockaddr;

  Client sender(1, sockaddr);
  sender.setPassword("senderP");
  sender.setUserName("senderU");
  sender.setNickname("senderN");

  Client receiver(2, sockaddr);
  receiver.setPassword("receiverP");
  receiver.setUserName("receiverU");
  receiver.setNickname("receiverN");

  Client random(3, sockaddr);
  random.setPassword("randomP");
  random.setUserName("randomU");
  random.setNickname("randomN");
  std::map<int, Client> myClients = {{1, sender}, {2, receiver}, {3, random}};
  std::map<std::string, Channel> myChannels;

  SECTION("PRIVMSG - Valid") {
    std::string response = ":senderN!senderU@" + serverHostname_g + " PRIVMSG receiverN :A valid message!\r\n";
    std::string msgWithoutParameters = "PRIVMSG receiverN :A valid message!";
    Message msg(msgWithoutParameters);
    Command cmd(msg, sender, myClients, password, serverStartTime, myChannels);
    std::vector<std::string>  param_ = msg.getParameters(); 
    REQUIRE(myClients.find(2)->second.getSendBuffer() == response);
    REQUIRE(sender.getSendBuffer() == "");
  }

  SECTION("PRIVMSG - no parameters -> 411 NORECIPIENT") {
    std::string response = ": 411 :No recipient given (privmsg)\r\n";
    std::string msgWithoutParameters = "PRIVMSG";
    Message msg(msgWithoutParameters);
    Command cmd(msg, sender, myClients, password, serverStartTime, myChannels);
    REQUIRE(sender.getSendBuffer() == response);
  }

  SECTION("PRIVMSG - too many parameters -> 407 TOOMANYTARGETS") {
    std::string response = ": 407 client2 :3 recipients. Only one target per message.\r\n";
    std::string msgWithTooManyTargets = "PRIVMSG client1 client2 client3 :message";
    Message msg(msgWithTooManyTargets);
    Command cmd(msg, sender, myClients, password, serverStartTime, myChannels);
    REQUIRE(sender.getSendBuffer() == response);
  }

  SECTION("PRIVMSG - 1 parameter -> 412 NOTEXTTOSEND") {
    std::string response = ": 412 :No text to send\r\n";
    std::string msgWithNoTextToSend = "PRIVMSG client";
    Message msg(msgWithNoTextToSend);
    Command cmd(msg, sender, myClients, password, serverStartTime, myChannels);
    REQUIRE(sender.getSendBuffer() == response);
  }

  SECTION("PRIVMSG - missing ':' for trailing parameter -> 412 NOTEXTTOSEND") {
    std::string response = ": 412 :No text to send\r\n";
    std::string msgWithNoTextToSend = "PRIVMSG sender message";
    Message msg(msgWithNoTextToSend);
    Command cmd(msg, sender, myClients, password, serverStartTime, myChannels);
    REQUIRE(sender.getSendBuffer() == response);
  }

  SECTION("PRIVMSG - non existing nickname -> 401 NOSUCHNICK") {
    std::string response = ": 401 blah :No such nick/channel\r\n";
    std::string msgWithNonExistingNickname = "PRIVMSG blah :a message";
    Message msg(msgWithNonExistingNickname);
    Command cmd(msg, sender, myClients, password, serverStartTime, myChannels);
    REQUIRE(sender.getSendBuffer() == response);
  }

}
