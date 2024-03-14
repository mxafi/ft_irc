#include "../catch2/catch_amalgamated.hpp"

#include <cerrno>
#include <map>
#include "../../src/client/Client.h"
#include "../../src/command/Command.h"
#include "../../src/message/Message.h"

extern std::string serverHostname_g;
std::string password = "password";

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
struct NicknameTestData {
    std::string description;
    std::string nickname;
    bool isValid;
};

std::vector<NicknameTestData> nicknameTestData = {{"Valid Nickname", "nick", true},
                                                  {"Truncated Nickname", "longerThanNineNick", true},
                                                  {"Contains a space ' '", "a nick", true},
                                                  {"Contains ','", "a,nick", false},
                                                  {"Empty ", "", false},
                                                  {"Contains Asterisk", "a*nick", false},
                                                  {"Contains '?'", "a?Nick", false},
                                                  {"contains '!' ", "a!nick", false},
                                                  {"Contains '@'", "a@nick", false},
                                                  {"Contains a '.'", "a.nick", false},
                                                  {"Contains '$'", "a$nick", false},
                                                  {"Contains a ':'", "a:nick", false},
                                                  {"Contains '&'", "a&nick", false}};

/**
*   Nick name validity is checked according to RFC2812 p. 7:
*   nickname   =  ( letter / special ) *8( letter / digit / special / "-" )
*   where,
*
*       special = %x5B-60 / %x7B-7D 
*       ; "[", "]", "\", "‘", "_", "^", "{", "|", "}"
*   
*   Exception is made for spaces which we accept as DALnet does, i.e. using the 
*   space as a delimiter setting the nick to the first delimited word.
*
*   A nickname can be of maximum 9 characters long
*/
TEST_CASE("Nick", "[command][nick]") {
    int errno_before = errno;
    REQUIRE(errno == errno_before);
    time_t serverStartTime = time(NULL);
    struct sockaddr sockaddr;
    std::map<std::string, Channel> myChannels;
    std::string response;

    Client client1(1, sockaddr);
    client1.setPassword("client1P");
    client1.setUserName("client1U");
    client1.setNickname("client1N");

    Client client2(2, sockaddr);
    client2.setPassword("client2P");
    client2.setUserName("client2U");
    client2.setNickname("client2N");
    std::map<int, Client> myClients = {{1, client1}, {2, client2}};

    GIVEN("A client with a nickname") {
        WHEN("Setting a valid nickname") {
            for (const auto& data : nicknameTestData) {
                if (data.isValid) {
                    std::string msg = "NICK " + data.nickname;
                    std::string truncatedNick =
                        data.nickname.substr(0, NICK_MAX_LENGTH_RFC2812);  // Truncate nick to autorized max nick length = 9 characters
                    size_t pos = truncatedNick.find_first_of(
                        " ");  // We decided to accept space as a delimiter as DalNet does, hence the two following functions
                    std::string expectedNick = truncatedNick.substr(0, pos);  // Split from white space

                    std::string originalNick = client1.getNickname();
                    Command cmd(msg, client1, myClients, password, serverStartTime, myChannels);
                    INFO(data.description + " failed to set nickname to: \"" + expectedNick + "\"");
                    REQUIRE(client1.getNickname() == expectedNick);
                    REQUIRE(client1.getNickname().length() <= NICK_MAX_LENGTH_RFC2812);
                }
            }
        }
        WHEN("Setting an invalid nickname") {
            for (const auto& data : nicknameTestData) {
                if (!data.isValid) {
                    if (data.nickname.length() == 0) {
                        response = ": 431 :No nickname given\r\n";
                    } else {
                        response = ": 432 " + data.nickname + " :Erroneous nickname\r\n";
                    }
                    std::string msg = "NICK " + data.nickname;
                    std::string truncatedNick = data.nickname.substr(0, NICK_MAX_LENGTH_RFC2812);
                    std::string originalNick = client1.getNickname();
                    client1.clearSendBuffer();
                    Command cmd(msg, client1, myClients, password, serverStartTime, myChannels);
                    INFO(data.description + " nickname should remained unchanged to: " + client1.getNickname());
                    REQUIRE(client1.getSendBuffer() == response);
                    REQUIRE(client1.getNickname() == originalNick);
                    REQUIRE(client1.getNickname().length() <= NICK_MAX_LENGTH_RFC2812);
                }
            }
        }
        WHEN("Setting a nickname that is already in use") {
            std::string originalNick = client1.getNickname();
            std::string existingNick = client2.getNickname();
            std::string msg = "NICK " + existingNick;
            response = ": 433 " + existingNick + " :Nickname is already in use\r\n";
            Command cmd(msg, client1, myClients, password, serverStartTime, myChannels);
            REQUIRE(client1.getSendBuffer() == response);
            REQUIRE(client1.getNickname() == originalNick);
        }
    }
}

TEST_CASE("Command PRIVMSG action", "[command][privmsg]") {
    int errno_before = errno;
    REQUIRE(errno == errno_before);
    time_t serverStartTime = time(NULL);
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
        std::string response = ":senderN!senderU@" + sender.getHost() + " PRIVMSG receiverN :A valid message!\r\n";
        std::string msgWithoutParameters = "PRIVMSG receiverN :A valid message!";
        Message msg(msgWithoutParameters);
        Command cmd(msg, sender, myClients, password, serverStartTime, myChannels);
        std::vector<std::string> param_ = msg.getParameters();
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
