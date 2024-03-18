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

    SECTION("INVITE specific definitions") {
        const std::string invitee = "oogabooga";
        const std::string actual_invite = INVITE(client_nick, client_user, client_host, invitee, channel);
        const std::string expected_invite = ":xuffy!markus@123.123.123.123 INVITE oogabooga :#test\r\n";
        REQUIRE(actual_invite == expected_invite);
    }

    SECTION("numeric replies 001-004") {
        const std::string actual_rpl_welcome_001 = RPL_WELCOME_001(server_name, client_nick, client_user, client_host);
        const std::string expected_rpl_welcome_001 =
            ":irc.example.com 001 xuffy :Welcome to the Internet Relay Network xuffy!markus@123.123.123.123\r\n";
        REQUIRE(actual_rpl_welcome_001 == expected_rpl_welcome_001);

        const std::string actual_rpl_yourhost_002 = RPL_YOURHOST_002(server_name, client_nick, version);
        const std::string expected_rpl_yourhost_002 = ":irc.example.com 002 xuffy :Your host is irc.example.com, running version 1.0\r\n";
        REQUIRE(actual_rpl_yourhost_002 == expected_rpl_yourhost_002);

        const std::string actual_rpl_created_003 = RPL_CREATED_003(server_name, client_nick, "2021-01-01");
        const std::string expected_rpl_created_003 = ":irc.example.com 003 xuffy :This server was created 2021-01-01\r\n";
        REQUIRE(actual_rpl_created_003 == expected_rpl_created_003);

        const std::string actual_rpl_myinfo_004 = RPL_MYINFO_004(server_name, client_nick, version, user_modes, channel_modes);
        const std::string expected_rpl_myinfo_004 = ":irc.example.com 004 xuffy irc.example.com 1.0 i b\r\n";
        REQUIRE(actual_rpl_myinfo_004 == expected_rpl_myinfo_004);
    }

    SECTION("numeric replies 324") {
        const std::string actual_rpl_channelmodeis_324 = RPL_CHANNELMODEIS_324(server_name, client_nick, channel, "b");
        const std::string expected_rpl_channelmodeis_324 = ":irc.example.com 324 xuffy #test +b\r\n";
        REQUIRE(actual_rpl_channelmodeis_324 == expected_rpl_channelmodeis_324);
    }

    SECTION("numeric replies 331-332") {
        const std::string actual_rpl_notopic_331 = RPL_NOTOPIC_331(server_name, client_nick, channel);
        const std::string expected_rpl_notopic_331 = ":irc.example.com 331 xuffy #test :No topic is set\r\n";
        REQUIRE(actual_rpl_notopic_331 == expected_rpl_notopic_331);

        const std::string actual_rpl_topic_332 = RPL_TOPIC_332(server_name, client_nick, channel, "Hello, world!");
        const std::string expected_rpl_topic_332 = ":irc.example.com 332 xuffy #test :Hello, world!\r\n";
        REQUIRE(actual_rpl_topic_332 == expected_rpl_topic_332);
    }

    SECTION("numeric replies 341") {
        const std::string actual_rpl_inviting_341 = RPL_INVITING_341(server_name, client_nick, "oogabooga", channel);
        const std::string expected_rpl_inviting_341 = ":irc.example.com 341 xuffy oogabooga #test\r\n";
        REQUIRE(actual_rpl_inviting_341 == expected_rpl_inviting_341);
    }

    SECTION("numeric replies 353") {
        const std::string actual_rpl_namreply_353 = RPL_NAMREPLY_353(server_name, client_nick, "=", channel, "xuffy markus");
        const std::string expected_rpl_namreply_353 = ":irc.example.com 353 xuffy = #test :xuffy markus\r\n";
        REQUIRE(actual_rpl_namreply_353 == expected_rpl_namreply_353);
    }

    SECTION("numeric replies 401") {
        const std::string actual_rpl_err_nosuchnick_401 = RPL_ERR_NOSUCHNICK_401(server_name, "xuffy");
        const std::string expected_rpl_err_nosuchnick_401 = ":irc.example.com 401 xuffy :No such nick/channel\r\n";
        REQUIRE(actual_rpl_err_nosuchnick_401 == expected_rpl_err_nosuchnick_401);
    }

    SECTION("numeric replies 403-405") {
        const std::string actual_rpl_err_nosuchchannel_403 = RPL_ERR_NOSUCHCHANNEL_403(server_name, channel);
        const std::string expected_rpl_err_nosuchchannel_403 = ":irc.example.com 403 #test :No such channel\r\n";
        REQUIRE(actual_rpl_err_nosuchchannel_403 == expected_rpl_err_nosuchchannel_403);

        const std::string actual_rpl_err_cannotsendtochan_404 = RPL_ERR_CANNOTSENDTOCHAN_404(server_name, channel);
        const std::string expected_rpl_err_cannotsendtochan_404 = ":irc.example.com 404 #test:Cannot send to channel\r\n";
        REQUIRE(actual_rpl_err_cannotsendtochan_404 == expected_rpl_err_cannotsendtochan_404);

        const std::string actual_rpl_err_toomanychannels_405 = RPL_ERR_TOOMANYCHANNELS_405(server_name, channel);
        const std::string expected_rpl_err_toomanychannels_405 = ":irc.example.com 405 #test :You have joined too many channels\r\n";
        REQUIRE(actual_rpl_err_toomanychannels_405 == expected_rpl_err_toomanychannels_405);
    }

    SECTION("numeric replies 407") {
        const std::string actual_rpl_err_toomanytargets_407 =
            RPL_ERR_TOOMANYTARGETS_407(server_name, "xuffy", "404", "Cannot send to channel");
        const std::string expected_rpl_err_toomanytargets_407 = ":irc.example.com 407 xuffy :404 recipients. Cannot send to channel\r\n";
        REQUIRE(actual_rpl_err_toomanytargets_407 == expected_rpl_err_toomanytargets_407);
    }

    SECTION("numeric replies 411-412") {
        const std::string actual_rpl_err_norecipient_411 = RPL_ERR_NORECIPIENT_411(server_name, "PRIVMSG");
        const std::string expected_rpl_err_norecipient_411 = ":irc.example.com 411 :No recipient given (PRIVMSG)\r\n";
        REQUIRE(actual_rpl_err_norecipient_411 == expected_rpl_err_norecipient_411);

        const std::string actual_rpl_err_notexttosend_412 = RPL_ERR_NOTEXTTOSEND_412(server_name);
        const std::string expected_rpl_err_notexttosend_412 = ":irc.example.com 412 :No text to send\r\n";
        REQUIRE(actual_rpl_err_notexttosend_412 == expected_rpl_err_notexttosend_412);
    }

    SECTION("numeric replies 421") {
        const std::string actual_rpl_err_unknowncommand_421 = RPL_ERR_UNKNOWNCOMMAND_421(server_name, "PRIVMSG");
        const std::string expected_rpl_err_unknowncommand_421 = ":irc.example.com 421 PRIVMSG :Unknown command\r\n";
        REQUIRE(actual_rpl_err_unknowncommand_421 == expected_rpl_err_unknowncommand_421);
    }

    SECTION("numeric replies 431-433") {
        const std::string actual_rpl_err_nonicknamegiven_431 = RPL_ERR_NONICKNAMEGIVEN_431(server_name);
        const std::string expected_rpl_err_nonicknamegiven_431 = ":irc.example.com 431 :No nickname given\r\n";
        REQUIRE(actual_rpl_err_nonicknamegiven_431 == expected_rpl_err_nonicknamegiven_431);

        const std::string actual_rpl_err_erroneousnickname_432 = RPL_ERR_ERRONEUSNICKNAME_432(server_name, "xuffy");
        const std::string expected_rpl_err_erroneousnickname_432 = ":irc.example.com 432 xuffy :Erroneous nickname\r\n";
        REQUIRE(actual_rpl_err_erroneousnickname_432 == expected_rpl_err_erroneousnickname_432);

        const std::string actual_rpl_err_nicknameinuse_433 = RPL_ERR_NICKNAMEINUSE_433(server_name, "xuffy");
        const std::string expected_rpl_err_nicknameinuse_433 = ":irc.example.com 433 xuffy :Nickname is already in use\r\n";
        REQUIRE(actual_rpl_err_nicknameinuse_433 == expected_rpl_err_nicknameinuse_433);
    }

    SECTION("numeric replies 441-442") {
        const std::string actual_rpl_err_usernotinchannel_441 = RPL_ERR_USERNOTINCHANNEL_441(server_name, "xuffy", channel);
        const std::string expected_rpl_err_usernotinchannel_441 = ":irc.example.com 441 xuffy #test :They aren't on that channel\r\n";
        REQUIRE(actual_rpl_err_usernotinchannel_441 == expected_rpl_err_usernotinchannel_441);

        const std::string actual_rpl_err_notonchannel_442 = RPL_ERR_NOTONCHANNEL_442(server_name, client_nick, channel);
        const std::string expected_rpl_err_notonchannel_442 = ":irc.example.com 442 xuffy #test :You're not on that channel\r\n";
        REQUIRE(actual_rpl_err_notonchannel_442 == expected_rpl_err_notonchannel_442);
    }

    SECTION("numeric replies 443") {
        const std::string actual_rpl_err_useronchannel_443 = RPL_ERR_USERONCHANNEL_443(server_name, client_nick, "oogabooga", channel);
        const std::string expected_rpl_err_useronchannel_443 = ":irc.example.com 443 xuffy oogabooga #test :is already on channel\r\n";
        REQUIRE(actual_rpl_err_useronchannel_443 == expected_rpl_err_useronchannel_443);
    }

    SECTION("numeric replies 451") {
        const std::string actual_rpl_err_notregistered_451 = RPL_ERR_NOTREGISTERED_451(server_name);
        const std::string expected_rpl_err_notregistered_451 = ":irc.example.com 451 :You have not registered\r\n";
        REQUIRE(actual_rpl_err_notregistered_451 == expected_rpl_err_notregistered_451);
    }

    SECTION("numeric replies 461-462") {
        const std::string actual_rpl_err_needmoreparams_461 = RPL_ERR_NEEDMOREPARAMS_461(server_name, client_nick, "PRIVMSG");
        const std::string expected_rpl_err_needmoreparams_461 = ":irc.example.com 461 xuffy PRIVMSG :Not enough parameters\r\n";
        REQUIRE(actual_rpl_err_needmoreparams_461 == expected_rpl_err_needmoreparams_461);

        const std::string actual_rpl_err_alreadyregistred_462 = RPL_ERR_ALREADYREGISTRED_462(server_name, client_nick);
        const std::string expected_rpl_err_alreadyregistred_462 = ":irc.example.com 462 xuffy :You may not reregister\r\n";
        REQUIRE(actual_rpl_err_alreadyregistred_462 == expected_rpl_err_alreadyregistred_462);
    }

    SECTION("numeric replies 467") {
        const std::string actual_rpl_err_keyset_467 = RPL_ERR_KEYSET_467(server_name, channel);
        const std::string expected_rpl_err_keyset_467 = ":irc.example.com 467 #test :Channel key already set\r\n";
        REQUIRE(actual_rpl_err_keyset_467 == expected_rpl_err_keyset_467);
    }

    SECTION("numeric replies 471-473") {
        const std::string actual_rpl_err_channelisfull_471 = RPL_ERR_CHANNELISFULL_471(server_name, client_nick, channel);
        const std::string expected_rpl_err_channelisfull_471 = ":irc.example.com 471 xuffy #test :Cannot join channel (+l)\r\n";
        REQUIRE(actual_rpl_err_channelisfull_471 == expected_rpl_err_channelisfull_471);

        const std::string actual_rpl_err_unknownmode_472 = RPL_ERR_UNKNOWNMODE_472(server_name, client_nick, "l", channel);
        const std::string expected_rpl_err_unknownmode_472 = ":irc.example.com 472 xuffy l :is unknown mode char to me for #test\r\n";
        REQUIRE(actual_rpl_err_unknownmode_472 == expected_rpl_err_unknownmode_472);

        const std::string actual_rpl_err_inviteonlychan_473 = RPL_ERR_INVITEONLYCHAN_473(server_name, client_nick, channel);
        const std::string expected_rpl_err_inviteonlychan_473 = ":irc.example.com 473 xuffy #test :Cannot join channel (+i)\r\n";
        REQUIRE(actual_rpl_err_inviteonlychan_473 == expected_rpl_err_inviteonlychan_473);
    }

    SECTION("numeric replies 475") {
        const std::string actual_rpl_err_badchannelkey_475 = RPL_ERR_BADCHANNELKEY_475(server_name, client_nick, channel);
        const std::string expected_rpl_err_badchannelkey_475 = ":irc.example.com 475 xuffy #test :Cannot join channel (+k)\r\n";
        REQUIRE(actual_rpl_err_badchannelkey_475 == expected_rpl_err_badchannelkey_475);
    }

    SECTION("numeric replies 482") {
        const std::string actual_rpl_err_chanoprivsneeded_482 = RPL_ERR_CHANOPRIVSNEEDED_482(server_name, client_nick, channel);
        const std::string expected_rpl_err_chanoprivsneeded_482 = ":irc.example.com 482 xuffy #test :You're not channel operator\r\n";
        REQUIRE(actual_rpl_err_chanoprivsneeded_482 == expected_rpl_err_chanoprivsneeded_482);
    }

    SECTION("numeric replies 501") {
        const std::string actual_rpl_err_umodeunknownflag_501 = RPL_ERR_UMODEUNKNOWNFLAG_501(server_name, client_nick);
        const std::string expected_rpl_err_umodeunknownflag_501 = ":irc.example.com 501 xuffy :Unknown MODE flag\r\n";
        REQUIRE(actual_rpl_err_umodeunknownflag_501 == expected_rpl_err_umodeunknownflag_501);
    }
}
