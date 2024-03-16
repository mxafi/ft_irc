#include "Command.h"

static bool isNextParamExist(unsigned int currentParamIndex, unsigned long numberOfParams) {
    return currentParamIndex < numberOfParams;
}

namespace irc {

// The MODE command is provided so that users may query and change the
// characteristics of a channel.  For more details on available modes
// and their uses, see "Internet Relay Chat: Channel Management" [IRC-
// CHAN].  Note that there is a maximum limit of three (3) changes per
// command for modes that take a parameter.
// MODE <channel> *( ( "-" / "+" ) *<modes> *<modeparams>
void Command::actionMode(Client& client) {
    unsigned long numberOfParams = param_.size();

    if (numberOfParams == 0) {
        client.appendToSendBuffer(RPL_ERR_NEEDMOREPARAMS_461(serverHostname_g, "MODE"));
        return;
    }

    // Now we know that we have at least one parameter

    // Check if the channel exists
    try {
        Channel& channel = allChannels_.at(param_.at(0));
        (void)channel;  // suppress unused variable warning
    } catch (const std::out_of_range& e) {
        client.appendToSendBuffer(RPL_ERR_NOSUCHCHANNEL_403(serverHostname_g, param_.at(0)));
        return;
    }

    // We know that the channel exists
    Channel& channel = allChannels_.at(param_.at(0));

    // Get the modes and return if the channel is the only parameter
    std::string enabledModes;
    if (numberOfParams == 1) {
        if (channel.isInviteOnly() == true) {
            enabledModes += "i";
        }
        if (channel.isTopicProtected() == true) {
            enabledModes += "t";
        }
        if (channel.getKey().empty() == false) {
            enabledModes += "k";
        }
        if (channel.getUserLimit() != CHANNEL_USER_LIMIT_DISABLED) {
            enabledModes += "l";
        }
        client.appendToSendBuffer(RPL_CHANNELMODEIS_324(serverHostname_g, channel.getName(), enabledModes));
        return;
    }

    // Now we know that we have at least two parameters

    // Check if the client is a channel operator, otherwise return
    if (channel.isOperator(client) == false) {
        client.appendToSendBuffer(RPL_ERR_CHANOPRIVSNEEDED_482(serverHostname_g, channel.getName()));
        return;
    }

    // Now we know that the client is a channel operator

    // Supported modes: i, t, k, l
    // i: invite-only (no parameters)
    // t: topic-protected (no parameters)
    // k: key (parameter: key)
    // l: user-limit (parameter: limit)

    // We will get something like: MODE #mychannel +ik key +tl 10

    int modeChangesWithParam = 0; // max is 3
    unsigned int currentParamIndex = 1; // max is numberOfParams - 1
    while (currentParamIndex < numberOfParams) {
        std::string& requestedModes = param_.at(currentParamIndex);
        char modifier = requestedModes[0];
        if (modifier != '+' && modifier != '-') {
            client.appendToSendBuffer(RPL_ERR_NEEDMOREPARAMS_461(serverHostname_g, "MODE")); // TODO: figure out correct reply
            return;
        }
        if (modifier == '+') {
            // TODO: enable the modes that are requested, only the last one can have a parameter, increment modeChangesWithParam (increment currentParamIndex if it has a parameter, use isNextParamExist)
        }
        if (modifier == '-') {
            // TODO: disable the modes that are requested, no parameters for now
        }

        currentParamIndex++;
    }

    // Missing: parsing and checking the modes and mode parameters (max 3 modes that take a parameter)
    // Missing: sending the mode changes to the channel members
}

}  // namespace irc