#include "../catch2/catch_amalgamated.hpp"
#include "../../src/message/Message.h"
#include "Message.h"

TEST_CASE("Message deserialization", "[Message]") {
    SECTION("Valid message with prefix and parameters") {
        std::string message = ":nick!~user@host.com PRIVMSG #channel :Hello, World!";
        Message msg(message);

        REQUIRE(msg.getPrefix() == "nick!~user@host.com");
        REQUIRE(msg.getCommand() == "PRIVMSG");
        REQUIRE(msg.getParameters().size() ==  2);
        REQUIRE(msg.getParameters()[0] == "#channel");
        REQUIRE(msg.getParameters()[1] == "Hello, World!");
    }

    SECTION("Valid message without prefix") {
        std::string message = "PRIVMSG #channel :Hello, World!";
        Message msg(message);

        REQUIRE(msg.getPrefix() == "");
        REQUIRE(msg.getCommand() == "PRIVMSG");
        REQUIRE(msg.getParameters().size() ==  2);
        REQUIRE(msg.getParameters()[0] == "#channel");
        REQUIRE(msg.getParameters()[1] == "Hello, World!");
    }

    SECTION("Invalid message with too many parameters") {
        std::string message = ":nick!~user@host.com PRIVMSG #channel :param1 param2 param3 param4 param5 param6 param7 param8 param9 param10 param11 param12 param13 param14 param15 param16";
        try {
            Message msg(message);
            FAIL("Expected an exception to be thrown");
        } catch (const std::runtime_error& e) {
            REQUIRE(std::string(e.what()) == "Too many parameters in message");
        }
    }

    SECTION("Empty message") {
        std::string message = "";
        try {
            Message msg(message);
            FAIL("Expected an exception to be thrown");
        } catch (const std::runtime_error& e) {
            REQUIRE(std::string(e.what()) == "Input line was too long,  0 instead of  512");
        }
    }

    SECTION("Malformed message") {
        std::string message = "PRIVMSG #channel :Hello, World! :extra";
        try {
            Message msg(message);
            FAIL("Expected an exception to be thrown");
        } catch (const std::runtime_error& e) {
            REQUIRE(std::string(e.what()) == "Malformed message");
        }
    }
}
