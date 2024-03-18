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


TEST_CASE("Command::execute tests", "[Command][execute]") {
    int errno_before = errno;
    REQUIRE (errno == errno_before);
    // Access the current test case data
    CommandTestFixture fixture;

    SECTION("Authenticated client executes a valid command") {
        // fixture.client.isAuthenticated(true);
        Message message("NICK newNick");
        Command cmd(message, fixture.client, fixture.clients, fixture.password, fixture.serverStartTime, fixture.channels);
        cmd.execute(fixture.client);
        // Verify that the command was executed successfully
        // This might involve checking the client's state or the send buffer
    }

    SECTION("Unauthenticated client executes a valid command") {
        // fixture.client.isAuthenticated(false);
        Message message("NICK newNick");
        Command cmd(message, fixture.client, fixture.clients, fixture.password, fixture.serverStartTime, fixture.channels);
        cmd.execute(fixture.client);
        // Verify that the command was not executed and an error message was sent
        // This might involve checking the client's send buffer
    }

    SECTION("Client executes an invalid command") {
        // fixture.client.isAuthenticated(true);
        Message message("INVALIDCOMMAND");
        Command cmd(message, fixture.client, fixture.clients, fixture.password, fixture.serverStartTime, fixture.channels);
        cmd.execute(fixture.client);
        // Verify that an error message was sent for the unknown command
        // This might involve checking the client's send buffer
    }

    SECTION("Client executes a command before authentication") {
        // fixture.client.isAuthenticated(false);
        Message message("NICK newNick");
        Command cmd(message, fixture.client, fixture.clients, fixture.password, fixture.serverStartTime, fixture.channels);
        cmd.execute(fixture.client);
        // Verify that an error message was sent for not being registered
        // This might involve checking the client's send buffer
    }

    SECTION("Client executes the CAP command") {
        // fixture.client.isAuthenticated(true);
        Message message("CAP");
        Command cmd(message, fixture.client, fixture.clients, fixture.password, fixture.serverStartTime, fixture.channels);
        cmd.execute(fixture.client);
        // Verify that the CAP command was ignored
        // This might involve checking the client's send buffer or state
    }
}
