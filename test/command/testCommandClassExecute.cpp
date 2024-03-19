#include "../catch2/catch_amalgamated.hpp"

#include <cerrno>
#include <map>
#include "../../src/client/Client.h"
#include "../../src/command/Command.h"
#include "../../src/message/Message.h"

using namespace irc;

struct CommandTestFixture {
    int dummyFd = 1;                // Dummy file descriptor
    struct sockaddr dummySockaddr;  // Dummy socket address
    Client client{dummyFd, dummySockaddr};
    std::map<int, Client> clients{{1, client}};
    std::map<std::string, Channel> channels;
    std::string password = "password";
    time_t serverStartTime = time(NULL);
};

/**
* This test case shows that numeric isnt used and probably should be considered to be removed.
* The  only place it may make sense to be used is in the command constructor, however, using 
* it to prevent execution of a command, will prevent to output the proper RFC error replies.
* It would though allow to skip executing commands that would be known in advance to fail.
*/
TEST_CASE("Command initialization", "[command]") {
    int errno_before = errno;
    std::string response;
    CommandTestFixture fixture;
    int expectedNumeric;
    std::string command;

    SECTION("Message numeric = 0 (default)") {
        expectedNumeric = 0;
        std::string nick = "newNick";
        Message msg("NICK " + nick);
        fixture.client.setPassword(fixture.password);
        fixture.client.setUserName("UserName");
        fixture.client.setNickname("UserNick");
        std::string oldNick = fixture.client.getNickname();
        Command cmd(msg, fixture.client, fixture.clients, fixture.password, fixture.serverStartTime, fixture.channels);
        REQUIRE(msg.getNumeric() == expectedNumeric);
        REQUIRE(fixture.client.getNickname() == nick);
        REQUIRE(errno == errno_before);  // numeric = 0, all good
    }

    SECTION("Message contains illegal nul character: numeric = 530") {
        int test_ERR_CUSTOM_ILLEGALNUL = 530;
        expectedNumeric = test_ERR_CUSTOM_ILLEGALNUL;
        Message msg(std::string("NICK N\0ick", 10));
        fixture.client.setPassword(fixture.password);
        fixture.client.setUserName("UserName");
        fixture.client.setNickname("UserNick");
        std::string oldNick = fixture.client.getNickname();
        Command cmd(msg, fixture.client, fixture.clients, fixture.password, fixture.serverStartTime, fixture.channels);
        REQUIRE(fixture.client.getNickname() == oldNick);
        REQUIRE(msg.getNumeric() == expectedNumeric);
        REQUIRE(errno == errno_before);
    }

    SECTION("Message contain too long input: numeric = 417") {
        int test_ERR_INPUTTOOLONG = 417;
        expectedNumeric = test_ERR_INPUTTOOLONG;
        Message msg(std::string("PRIVMSG " + std::string(1000, 'a')));
        std::string censorTooLongMsg = ": 412 :No text to send\r\n";  // error returned by PRIVMSG
        fixture.client.setPassword(fixture.password);
        fixture.client.setUserName("UserName");
        fixture.client.setNickname("UserNick");
        std::string oldNick = fixture.client.getNickname();
        fixture.client.clearSendBuffer();
        Command cmd(msg, fixture.client, fixture.clients, fixture.password, fixture.serverStartTime, fixture.channels);
        REQUIRE(msg.getNumeric() == expectedNumeric);  // TODO: evaluate the need to keep numeric. It isnt used only as a setter.
        REQUIRE(fixture.client.getSendBuffer() == censorTooLongMsg);
        REQUIRE(errno == errno_before);
    }
}

TEST_CASE("Command constructor validation tests", "[Command][constructor][validation]") {
    int dummyFd = 1;
    struct sockaddr dummySockaddr;
    Client client(dummyFd, dummySockaddr);
    std::map<int, Client> allClients{{1, client}};
    std::map<std::string, Channel> allChannels;
    std::string password = "password";
    time_t serverStartTime = time(NULL);

    // Test with valid inputs
    Message message("NICK newNick");
    REQUIRE_NOTHROW(Command(message, client, allClients, password, serverStartTime, allChannels));

    // Test with invalid serverStartTime
    serverStartTime = 0;
    REQUIRE_THROWS_AS(Command(message, client, allClients, password, serverStartTime, allChannels), std::invalid_argument);

    // Test with empty password
    serverStartTime = time(NULL);  // Reset to valid value
    password = "";
    REQUIRE_THROWS_AS(Command(message, client, allClients, password, serverStartTime, allChannels), std::invalid_argument);
}

TEST_CASE("Command::execute tests", "[Command][execute]") {
    int errno_before = errno;
    REQUIRE(errno == errno_before);
    std::string response;
    CommandTestFixture fixture;
    std::string command;

    /**
      * @test Tests that an authenticated client can execute a valid command.
      * This test simulates a client that has already authenticated by setting a password,
      * username, and nickname. It then sends a NICK command to change the nickname.
      * The expected behavior is that the command is executed successfully, and the client's
      * nickname is updated accordingly.
      */
    SECTION("Authenticated client executes a valid command") {
        fixture.client.setPassword(fixture.password);
        fixture.client.setUserName("UserName");
        fixture.client.setNickname("UserNick");
        std::string oldNick = fixture.client.getNickname();

        fixture.client.clearSendBuffer();
        command = "NICK";
        Message message(command + " newNick");
        Command cmd(message, fixture.client, fixture.clients, fixture.password, fixture.serverStartTime, fixture.channels);
        response = ":" + oldNick + "!" + fixture.client.getUserName() + "@" + fixture.client.getHost() + " " + command + " " +
                   fixture.client.getNickname() + "\r\n";
        REQUIRE(fixture.client.getSendBuffer() == response);
    }
    /**
      * @test Tests that an unauthenticated client cannot execute a valid command.
      * This test simulates a client that has not yet authenticated by not setting a password.
      * It then attempts to send a NICK command. The expected behavior is that the command
      * is not executed, and an error message is sent to the client indicating that a password
      * must be sent first.
      */
    SECTION("Unauthenticated client executes a valid command") {
        fixture.client.clearSendBuffer();
        command = "NICK";
        Message message(command + " firstNick");
        Command cmd(message, fixture.client, fixture.clients, fixture.password, fixture.serverStartTime, fixture.channels);
        response = "ERROR :You must send a password first\r\n"; // Custom made error message to handle our custom authentication
        REQUIRE(fixture.client.getSendBuffer() == response);
        REQUIRE(fixture.client.getWantDisconnect() == TRUE);    // if unauthenticated a user is disconnected
    }

    SECTION("Client executes an invalid command") {
        fixture.client.setPassword(fixture.password);
        fixture.client.setUserName("UserName");
        fixture.client.setNickname("UserNick");
        Message message("INVALIDCOMMAND");
        Command cmd(message, fixture.client, fixture.clients, fixture.password, fixture.serverStartTime, fixture.channels);
        response = ":" ;
        REQUIRE(fixture.client.getSendBuffer() == response);
        
    }

    SECTION("Client executes a command before authentication") {
        Message message("NICK newNick");
        Command cmd(message, fixture.client, fixture.clients, fixture.password, fixture.serverStartTime, fixture.channels);
        cmd.execute(fixture.client);
    }

    SECTION("Client executes the CAP command") {
        Message message("CAP");
        Command cmd(message, fixture.client, fixture.clients, fixture.password, fixture.serverStartTime, fixture.channels);
        cmd.execute(fixture.client);
    }
}
