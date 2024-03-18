#include "Command.h"

namespace irc {

void Command::actionPrivmsg(Client& client) {
    size_t amountParameters = param_.size();
    if (amountParameters == 0) {
        client.appendToSendBuffer(RPL_ERR_NORECIPIENT_411(serverHostname_g, "privmsg"));
        LOG_DEBUG("CMD::PRIVMSG::NO RECIPIENT and NO MESSAGE");
        return;
    }
    if (amountParameters > 2) {
        client.appendToSendBuffer(
            RPL_ERR_TOOMANYTARGETS_407(serverHostname_g, param_.at(1), std::to_string(param_.size() - 1), "Only one target per message."));
        LOG_DEBUG("CMD::PRIVMSG::TOO MANY TARGETS");
        return;
    }
    if (amountParameters == 1) {
        client.appendToSendBuffer(RPL_ERR_NOTEXTTOSEND_412(serverHostname_g));
        LOG_DEBUG("CMD::PRIVMSG::NO TEXT (1 param only)");
        return;
    }

    // Now we know we have exactly 2 parameters
    std::string targetParam = param_.at(0);
    std::string messageParam = param_.at(1);

    // Required format: PRIVMSG <target> :<message> (checking the colon here)
    if (messageParam.front() != ':') {
        client.appendToSendBuffer(RPL_ERR_NOTEXTTOSEND_412(serverHostname_g));
        LOG_DEBUG("CMD::PRIVMSG::NO TEXT (2nd param missing colon)");
        return;
    }
    std::string messageParamWithoutColon = messageParam.erase(0, 1);

    std::string channelPattern = CHANNEL_PREFIXES;
    if (channelPattern.find(targetParam.front()) != std::string::npos) {
        LOG_DEBUG("CMD::PRIVMSG: targetParam is a channel");
        try {
            Channel& channel = allChannels_.at(targetParam);
            LOG_DEBUG("Command::actionPrivmsg: Channel found " + targetParam);
            if (channel.isMember(client) == false) {
                client.appendToSendBuffer(RPL_ERR_NOTONCHANNEL_442(serverHostname_g, targetParam));
                return;
            }
            LOG_DEBUG("Command::actionPrivmsg: nick " + client.getNickname() + " is a member of " + targetParam);
            channel.sendMessageToMembersExcluding(COM_MESSAGE(client.getNickname(), client.getUserName(), client.getHost(), "PRIVMSG",
                                                              targetParam + " :" + messageParamWithoutColon),
                                                  client);
        } catch (std::out_of_range& e) {
            LOG_DEBUG("CMD::PRIVMSG::CHANNEL NOT FOUND");
            client.appendToSendBuffer(RPL_ERR_NOSUCHNICK_401(serverHostname_g, targetParam));
        }
        return;
    }

    LOG_DEBUG("CMD::PRIVMSG: targetParam is a user (nick)");
    try {
        Client& targetClient = findClientByNicknameOrThrow(targetParam);
        LOG_DEBUG("CMD::PRIVMSG: Message is :" + messageParamWithoutColon + " from " + client.getNickname() + " to " + targetParam);

        std::string formattedSender = FORMAT_NICK_USER_HOST(client.getNickname(), client.getUserName(), client.getHost());
        std::string privmsg = PRIVMSG_FORMAT(formattedSender, targetParam, messageParamWithoutColon);
        targetClient.appendToSendBuffer(privmsg);
    } catch (std::out_of_range& e) {
        client.appendToSendBuffer(RPL_ERR_NOSUCHNICK_401(serverHostname_g, targetParam));
        LOG_DEBUG("CMD::PRIVMSG::findClientByNickname: USER NOT FOUND");
        return;
    }
}

}  // namespace irc
