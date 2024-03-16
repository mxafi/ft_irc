#include "../catch2/catch_amalgamated.hpp"

#include "../../src/common/reply.h"

TEST_CASE("Replies are known valid strings", "[reply]") {
    const std::string server_name = "irc.example.com";
    const std::string client_nick = "xuffy";
    const std::string client_user = "markus";
    const std::string client_host = "123.123.123.123";
    const std::string version = "1.0";
    const std::string user_modes = "i";
    const std::string channel_modes = "b";
    const std::string channel = "#test";

    SECTION("meta definitions") {
      // RPL_META_MESSAGE
      const std::string message = "Hello, world!";
      const std::string numeric = "060";
      const std::string actual_meta_message = RPL_META_MESSAGE(server_name, numeric, message);
      const std::string expected_meta_message = ":irc.example.com 060 Hello, world!\r\n";
      REQUIRE(actual_meta_message == expected_meta_message);
      // ERR_MESSAGE
      const std::string actual_err_message = ERR_MESSAGE(message);
      const std::string expected_err_message = "ERROR :Hello, world!\r\n";
      REQUIRE(actual_err_message == expected_err_message);
      // COM_MESSAGE
      const std::string command = "PRIVMSG";
      const std::string params = "Hello, world!";
      const std::string actual_com_message = COM_MESSAGE(client_nick, client_user, client_host, command, params);
      const std::string expected_com_message = ":xuffy!markus@123.123.123.123 PRIVMSG Hello, world!\r\n";
      REQUIRE(actual_com_message == expected_com_message);
      // RPL_MESSAGE
      const std::string actual_rpl_message = RPL_MESSAGE(message);
      const std::string expected_rpl_message = "Hello, world!\r\n";
    }
    SECTION("PRIVMSG specific definitions") {
      // FORMAT_NICK_USER_HOST
      const std::string actual_nick_user_host = FORMAT_NICK_USER_HOST(client_nick, client_user, client_host);
      const std::string expected_nick_user_host = ":xuffy!markus@123.123.123.123";
      REQUIRE(actual_nick_user_host == expected_nick_user_host);
      // PRIVMSG_FORMAT
      const std::string formatted_sender = FORMAT_NICK_USER_HOST(client_nick, client_user, client_host);
      const std::string text = "Hello, world!";
      const std::string actual_privmsg_format = PRIVMSG_FORMAT(formatted_sender, channel, text);
      const std::string expected_privmsg_format = ":xuffy!markus@123.123.123.123 PRIVMSG #test :Hello, world!\r\n";
      REQUIRE(actual_privmsg_format == expected_privmsg_format);
    }
    SECTION("numeric replies 0-299") {}
    SECTION("numeric replies 300-399") {}
    SECTION("numeric replies 400-499") {}
}
