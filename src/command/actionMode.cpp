#include "Command.h"

static bool isModeSupported(char mode) {
    return std::string(SUPPORTED_CHANNEL_MODES).find(mode) != std::string::npos;
}

static bool isModeWithParam(char mode) {
    return std::string(SUPPORTED_CHANNEL_MODES_WITH_PARAM).find(mode) != std::string::npos;
}

static bool isNextParamExist(unsigned int currentParamIndex, unsigned long numberOfParams) {
    bool ret;
    if (numberOfParams > 0) {
        ret = currentParamIndex < numberOfParams - 1;
    } else {
        ret = currentParamIndex < numberOfParams;
    }
    return ret;
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
        client.appendToSendBuffer(RPL_ERR_NEEDMOREPARAMS_461(serverHostname_g, client.getNickname(), "MODE"));
        return;
    }

    // Now we know that we have at least one parameter

    // Handle actionMode for a user
    if (std::string(CHANNEL_PREFIXES).find(param_.at(0).front()) == std::string::npos) {
        // We know that the first parameter is not a channel
        client.appendToSendBuffer(RPL_ERR_UMODEUNKNOWNFLAG_501(serverHostname_g, client.getNickname()));  // TODO: placeholder
        // TODO: implement user mode changes
        return;
    }

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
        client.appendToSendBuffer(RPL_CHANNELMODEIS_324(serverHostname_g, client.getNickname(), channel.getName(), enabledModes));
        return;
    }

    // Now we know that we have at least two parameters

    // Check if the client is a channel operator, otherwise return
    if (channel.isOperator(client) == false) {
        client.appendToSendBuffer(RPL_ERR_CHANOPRIVSNEEDED_482(serverHostname_g, client.getNickname(), channel.getName()));
        return;
    }

    // Now we know that the client is a channel operator

    // Supported modes: i, t, k, o, l
    // i: invite-only (no parameter)
    // t: topic-protected (no parameter)
    // k: key (parameter: key)
    // o: operator (parameter: nickname)
    // l: user-limit (parameter: limit)

    // We will get something like: MODE #mychannel +ik key +tl 10
    // We will parse it into a vector of modestructs like this:
    // vector[0]: {modifier: '+', mode: 'i', param: ""}
    // vector[1]: {modifier: '+', mode: 'k', param: "key"}
    // vector[2]: {modifier: '+', mode: 't', param: ""}
    // vector[3]: {modifier: '+', mode: 'l', param: "10"}

    std::vector<Channel::modestruct> modeRequests;
    int modeChangesWithParam = 0;        // max is 3
    unsigned int currentParamIndex = 1;  // max is numberOfParams - 1
    while (currentParamIndex < numberOfParams) {
        Channel::modestruct currentMode;
        std::string& currentParamString = param_.at(currentParamIndex);
        char currentModifier = currentParamString[0];
        if (currentModifier != '+' && currentModifier != '-') {
            client.appendToSendBuffer(
                RPL_ERR_NEEDMOREPARAMS_461(serverHostname_g, client.getNickname(), "MODE"));  // TODO: figure out correct reply
            return;
        }
        currentParamString = currentParamString.substr(1);
        for (char mode : currentParamString) {
            if (isModeSupported(mode) == false) {
                client.appendToSendBuffer(RPL_ERR_UNKNOWNMODE_472(serverHostname_g, client.getNickname(), mode, channel.getName()));
                return;
            }
            currentMode.modifier = currentModifier;
            currentMode.mode = mode;
            modeRequests.push_back(currentMode);
        }

        if (isNextParamExist(currentParamIndex, numberOfParams) && isModeWithParam(modeRequests.back().mode)) {
            modeChangesWithParam++;
            currentParamIndex++;
            modeRequests.back().param = param_.at(currentParamIndex);
        }
        currentParamIndex++;
    }
    if (modeChangesWithParam > 3) {
        client.appendToSendBuffer(
            RPL_ERR_NEEDMOREPARAMS_461(serverHostname_g, client.getNickname(), "MODE"));  // TODO: figure out correct reply
        return;
    }

    // Now we have a vector of modeRequests, with valid modes but possibly invalid parameters

    // Actually go through the modeRequests, check parameters, change the modes, and send the changes to the channel members
    for (Channel::modestruct& modeRequest : modeRequests) {
        channel.handleModeChange(client, modeRequest);
    }
}

}  // namespace irc
