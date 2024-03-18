#include "../catch2/catch_amalgamated.hpp"

#include <cerrno>
#include <string>
#include "../../src/common/magicNumber.h"
#include "../../src/server/Server.h"

TEST_CASE("server is started properly", "[server]") {
    errno = 0;
    char port[] = "6677"; // 6667 is the default port for IRC, this is a test port to avoid conflicts
    std::string password = "horse";
    irc::Server server(port, password);

    REQUIRE(isServerRunning_g == false);
    int startValue = server.start();
    REQUIRE(startValue == 0);  // 0 is the value of SUCCESS
    REQUIRE(isServerRunning_g == true);
    REQUIRE(serverHostname_g.empty() == false);
    REQUIRE(server.getServerSocketFd() > 0);  // socket() returns a positive int
    REQUIRE(server.getServerSocketDomain() == AF_INET);
    REQUIRE(server.getServerSocketType() == SOCK_STREAM);
    REQUIRE(server.getServerSocketProtocol() == DEFAULT_SOCKET_PROTOCOL);
    REQUIRE(server.getServerInfo().ai_family == AF_INET);
    REQUIRE(server.getServerInfo().ai_socktype == SOCK_STREAM);
    REQUIRE(server.getServerInfo().ai_protocol == TCP_PROTOCOL);
    REQUIRE(server.getPassword() == password);
    REQUIRE(server.getPort() == port);
    REQUIRE(errno == 0);
}
