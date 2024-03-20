#include "../catch2/catch_amalgamated.hpp"

#include <cerrno>
#include <map>
#include "../../src/client/Client.h"
#include "../../src/command/Command.h"
#include "../../src/message/Message.h"

int errno_before;
extern std::string serverHostname_g;

using namespace irc;

/****
    * This test case shows that numeric isnt used and probably should be considered to be removed.
    * The  only place it may make sense to be used is in the command constructor, however, using 
    * it to prevent execution of a command, will prevent to output the proper RFC error replies.
    * It would though allow to skip executing commands that would be known in advance to fail.
    */
TEST_CASE("Command initialization", "[command][initialization]") {
    int dummyFd = 1;

    struct sockaddr sockaddr {};

    Client client(dummyFd, sockaddr);
    std::map<int, Client> clients{{1, client}};
    std::map<std::string, Channel> channels;
    time_t serverStartTime = time(NULL);
    std::string password = "password";
    errno_before = errno;
    std::string response;
    int expectedNumeric;
    std::string command;

    SECTION("Message numeric = 0 (default)") {
        expectedNumeric = 0;
        std::string nick = "newNick";
        Message msg("NICK " + nick);
        client.setPassword(password);
        client.setUserName("UserName");
        client.setNickname("UserNick");
        std::string oldNick = client.getNickname();
        Command cmd(msg, client, clients, password, serverStartTime, channels);
        REQUIRE(msg.getNumeric() == expectedNumeric);
        REQUIRE(client.getNickname() == nick);
        REQUIRE(errno == errno_before);
    }

    SECTION("Message contains illegal nul character: numeric = 530") {
        int test_ERR_CUSTOM_ILLEGALNUL = 530;
        expectedNumeric = test_ERR_CUSTOM_ILLEGALNUL;
        Message msg(std::string("NICK N\0ick", 10));
        client.setPassword(password);
        client.setUserName("UserName");
        client.setNickname("UserNick");
        std::string oldNick = client.getNickname();
        Command cmd(msg, client, clients, password, serverStartTime, channels);
        REQUIRE(client.getNickname() == oldNick);
        REQUIRE(msg.getNumeric() == expectedNumeric);
        REQUIRE(errno == errno_before);
    }

    SECTION("Message contain too long input: numeric = 417") {
        int test_ERR_INPUTTOOLONG = 417;
        expectedNumeric = test_ERR_INPUTTOOLONG;
        Message msg(std::string("PRIVMSG " + std::string(1000, 'a')));
        std::string censorTooLongMsg = ": 412 :No text to send\r\n";  // error returned by PRIVMSG
        client.setPassword(password);
        client.setUserName("UserName");
        client.setNickname("UserNick");
        std::string oldNick = client.getNickname();
        client.clearSendBuffer();
        Command cmd(msg, client, clients, password, serverStartTime, channels);
        REQUIRE(msg.getNumeric() == expectedNumeric);  // TODO: evaluate the need to keep numeric. It is used only as a setter.
        REQUIRE(client.getSendBuffer() == censorTooLongMsg);
        REQUIRE(errno == errno_before);
    }
}

/****
    * @brief Test cases for validating Command constructor.
    *
    * This test case verifies the behavior of the Command constructor with different inputs.
    */
TEST_CASE("Command constructor validation tests", "[Command][constructorValidation]") {
    errno_before = errno;
    int dummyFd = 1;

    struct sockaddr sockaddr {};

    Client client(dummyFd, sockaddr);
    std::map<int, Client> allClients{{1, client}};
    std::map<std::string, Channel> allChannels;
    std::string password = "password";
    time_t serverStartTime = time(NULL);
    Message message("NICK newNick");  // Test with valid inputs
    REQUIRE_NOTHROW(Command(message, client, allClients, password, serverStartTime, allChannels));
    serverStartTime = 0;  // Test with invalid serverStartTime
    REQUIRE_THROWS_AS(Command(message, client, allClients, password, serverStartTime, allChannels), std::invalid_argument);
    // Test with empty password
    serverStartTime = time(NULL);  // Reset to valid value
    password = "";
    REQUIRE_THROWS_AS(Command(message, client, allClients, password, serverStartTime, allChannels), std::invalid_argument);
    REQUIRE(errno == errno_before);
}

/****
    * @brief Helper function to execute a command and validate the response made in Command Execute tests
    *
    * This function executes a command, validates its response, and checks whether the client wants to disconnect.
    * @param client The client object.
    * @param commandStr The command string to execute.
    * @param expectedResponse The expected response from the server.
    * @param expectDisconnect Whether the client expects to disconnect after executing the command when unregistered.
    */
void executeAndValidateCommand(Client& client, const std::string& commandStr, const std::string& expectedResponse, bool expectDisconnect) {
    time_t serverStartTime = time(NULL);
    client.clearSendBuffer();
    Message message(commandStr);
    std::string password = "password";
    std::map<int, Client> myClients = {{1, client}};
    std::map<std::string, Channel> myChannels;
    Command cmd(message, client, myClients, password, serverStartTime, myChannels);
    REQUIRE(errno_before == errno);
    REQUIRE(client.getSendBuffer() == expectedResponse);
    REQUIRE(client.getWantDisconnect() == expectDisconnect);
}

/****
    * @brief Test cases for executing commands.
    *
    * This test case verifies the behavior of executing different commands with various unauthenticated, partially authenticated and authenticated clients
    */
TEST_CASE("Command::execute tests", "[Command][execute]") {
    errno_before = errno;
    int dummyFd = 1;

    struct sockaddr sockaddr {};

    Client client(dummyFd, sockaddr);

    SECTION("Authenticated client executes a valid command") {
        client.setPassword("password");
        client.setUserName("UserName");
        client.setNickname("UserNick");
        executeAndValidateCommand(client, "NICK newNick", ":UserNick!UserName@" + client.getHost() + " NICK newNick\r\n", false);
    }

    SECTION("Partially authenticated client (pass + username) executes a valid command") {
        client.setPassword("password");
        client.setUserName("UserName");
        executeAndValidateCommand(client, "PRIVMSG :This is a message", ": 451 :You have not registered\r\n", false);
    }

    SECTION("Partially authenticated client (pass + nick) executes a valid command") {
        client.setPassword("password");
        client.setNickname("UserNick");
        executeAndValidateCommand(client, "PRIVMSG :This is a message", ": 451 :You have not registered\r\n", false);
    }

    SECTION("Unauthenticated client executes NICK before PASS") {
        executeAndValidateCommand(client, "NICK firstNick", "ERROR :You must send a password first\r\n", true);
    }

    SECTION("Unauthenticated client executes USER before PASS") {
        executeAndValidateCommand(client, "USER username", "ERROR :You must send a password first\r\n", true);
    }

    SECTION("Unauthenticated client executes a valid command (other than PASS or NICK) before PASS") {
        executeAndValidateCommand(client, "PRIVMSG toto :Hello!", ": 451 :You have not registered\r\n", false);
    }

    SECTION("Authenticated client executes an empty string \"\" command") {
        client.setPassword("password");
        client.setUserName("UserName");
        client.setNickname("UserNick");
        executeAndValidateCommand(client, "", ": 421  :Unknown command\r\n", false);
    }

    SECTION("Unauthenticated client executes an empty string \"\" command") {
        executeAndValidateCommand(client, "", ": 451 :You have not registered\r\n", false);
    }

    SECTION("Authenticated client executes a white spaced command") {
        client.setPassword("password");
        client.setUserName("UserName");
        client.setNickname("UserNick");
        executeAndValidateCommand(client, R"(\f\r\v\n\t)", ": 421 \\f\\r\\v\\n\\t :Unknown command\r\n", false);
    }

    SECTION("Unauthenticated client executes a white spaced command") {
        executeAndValidateCommand(client, R"(\f\r\v\n\t)", ": 451 :You have not registered\r\n", false);
    }

    SECTION("Authenticated client executes a command including white spaces and normal character") {
        client.setPassword("password");
        client.setUserName("UserName");
        client.setNickname("UserNick");
        executeAndValidateCommand(client, R"(\f\rc\vm\nd\t)", ": 421 \\f\\rc\\vm\\nd\\t :Unknown command\r\n", false);
    }

    SECTION("Unauthenticated client executes a command including white spaces and normal character") {
        executeAndValidateCommand(client, R"(\f\rc\vm\nd\t)", ": 451 :You have not registered\r\n", false);
    }

    SECTION("Authenticated client executes a command containing special characters") {
        client.setPassword("password");
        client.setUserName("UserName");
        client.setNickname("UserNick");
        executeAndValidateCommand(client, "@?!$:&", ": 421 @?!$:& :Unknown command\r\n", false);
    }

    SECTION("Unauthenticated client executes a command containing special characters") {
        executeAndValidateCommand(client, "@?!$:&", ": 451 :You have not registered\r\n", false);
    }

    SECTION("Authenticated Client executes an invalid command") {
        client.setPassword("password");
        client.setUserName("UserName");
        client.setNickname("UserNick");
        executeAndValidateCommand(client, "INVALIDCOMMAND", ": 421 INVALIDCOMMAND :Unknown command\r\n", false);
    }

    SECTION("Authenticated Client executes the CAP command before being authenticated") {
        executeAndValidateCommand(client, "CAP", "", false);
    }

    SECTION("Authenticated Client executes the CAP command after being authenticated") {
        client.setPassword("password");
        client.setUserName("UserName");
        client.setNickname("UserNick");
        executeAndValidateCommand(client, "CAP", ": 421 CAP :Unknown command\r\n", false);
    }
}
