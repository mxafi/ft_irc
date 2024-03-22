#include "../catch2/catch_amalgamated.hpp"

#include <cerrno>
#include <map>
#include "../../src/client/Client.h"
#include "../../src/command/Command.h"
#include "../../src/message/Message.h"

extern std::string serverHostname_g;
std::string password = "password";
const int test_nick_max_length_rfc2812 = 9; // Define the maximum nickname length locally

using namespace irc;

struct NicknameTestData {
    std::string description;
    std::string nickname;
    bool isValid;
};

std::vector<NicknameTestData> nicknameTestData = {
    //valid nicknames
    {"Valid Nickname", "nick", true},
    {"Truncated Nickname", "longerThanNineCharNick", true},
    {"Contains a space ' '", "a nick", true},
    {"Contains a mix of valid and invalid characters with spaces", "a *nick?!@. ", true},
    {"Trailing whitespace", "nick   ", true},
    {"Leading and trailing whitespace", "   nick    ", true},
    {"Leading whitespace", "     nick", true},
    {"Very long string", std::string(10000, 'a'), true},
    {"Contains multiple words delimited by whitespaces", R"(abc\rb\nc\td )", true},
    {"Contains multiple words delimited by whitespaces", R"(a\tb\nc\rd )", true},
    {"Contains multiple words delimited by spaces", " a b c d e f g h i j k l m n o p q r s t u v w x y z ", true},
    //invalid nicknames
    {"Very long string of spaces", std::string(10000, ' '), false},
    {"Contains ','", "a,nick", false},
    {"Contains only spaces and tab", " \t ", false},
    {"Contains form feed", "\f", false},
    {"Contains line feed", "\n", false},
    {"Contains carriage return", "\r", false},
    {"Contains horizontal tab", "\t", false},
    {"Contains vertical tab", "\v", false},
    {"Empty", "", false},
    {"Contains Asterisk", "a*nick", false},
    {"Contains '?'", "a?Nick", false},
    {"contains '!'", "a!nick", false},
    {"Contains '@'", "a@nick", false},
    {"Contains a '.'", "a.nick", false},
    {"Contains '$'", "a$nick", false},
    {"Contains a ':'", "a:nick", false},
    {"Contains '&'", "a&nick", false},
    {"Contains a mix of valid and invalid characters", "a*nick?!@.", false},
    {"Contains a mix of valid and invalid characters with line feeds", "a\n*nick?!@.\n", false},
    {"Contains a mix of valid and invalid characters with carriage returns", "a\r*nick?!@.\r", false},
    {"Contains a mix of valid and invalid characters with tabs", "a\t*nick?!@.\t", false},
    {"Contains null character", std::string("a\0nick", 6), false},
    {"Contains Unicode character", "a\u00E9nick", false},       // Example with a Unicode character (é)
    {"Contains extended ASCII character", "a\x80nick", false},  // Example with an extended ASCII character
    {"Contains non-printable character", "a\001nick", false},   // Example with a non-printable character (ASCII SOH)
    {"Contains invisible character", "a\u200Bnick", false},     // Example with an invisible character (zero-width space)
};

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
                        data.nickname.substr(0, test_nick_max_length_rfc2812);  // Truncate nick to autorized max nick length = 9 characters
                    // Trim leading and trailing spaces
                    size_t start = truncatedNick.find_first_not_of(" ");
                    size_t end = truncatedNick.find_last_not_of(" ");
                    std::string trimmedNick = (start == std::string::npos) ? "" : truncatedNick.substr(start, end - start + 1);

                    // Find the first whitespace character in the trimmed string
                    size_t pos = trimmedNick.find_first_of(" ");
                    std::string expectedNick;

                    // If a whitespace character is found, extract the substring from the start to the first whitespace character
                    if (pos != std::string::npos) {
                        expectedNick = trimmedNick.substr(0, pos);
                    } else {
                        // If no whitespace character is found, set expectedNick to the entire trimmed string
                        expectedNick = trimmedNick;
                    }
                    std::string originalNick = client1.getNickname();
                    client1.clearSendBuffer();
                    Command cmd(msg, client1, myClients, password, serverStartTime, myChannels);
                    INFO(data.description + " failed to set nickname to: \"" + expectedNick + "\" trimmed nick: \"" + trimmedNick + "\"");
                    REQUIRE(client1.getNickname() == expectedNick);
                    REQUIRE(client1.getNickname().length() <= test_nick_max_length_rfc2812);
                }
            }
        }
        WHEN("Setting an invalid nickname") {
            for (const auto& data : nicknameTestData) {
                if (!data.isValid) {
                    if (data.nickname.length() == 0 || std::all_of(data.nickname.begin(), data.nickname.end(), ::isspace)) {
                        response = ": 431 :No nickname given\r\n";
                    } else {
                        response = ": 432 " + data.nickname + " :Erroneous nickname\r\n";
                    }
                    std::string msg = "NICK " + data.nickname;
                    std::string truncatedNick = data.nickname.substr(0, test_nick_max_length_rfc2812);
                    std::string originalNick = client1.getNickname();
                    client1.clearSendBuffer();
                    Command cmd(msg, client1, myClients, password, serverStartTime, myChannels);
                    INFO(data.description + " nickname should remained unchanged to: \"" + originalNick + "\" but got: \"" +
                         client1.getNickname() + "\" instead");
                    REQUIRE(client1.getSendBuffer() == response);
                    REQUIRE(client1.getNickname() == originalNick);
                    REQUIRE(client1.getNickname().length() <= test_nick_max_length_rfc2812);
                }
            }
        }
        WHEN("Setting a nickname that is already in use") {
            std::string originalNick = client1.getNickname();
            std::string existingNick = client2.getNickname();
            std::string msg = "NICK " + existingNick;
            response = ": 433 " + originalNick + " " + existingNick + " :Nickname is already in use\r\n";
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
