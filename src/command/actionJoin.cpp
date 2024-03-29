#include "Command.h"

namespace irc {

void Command::actionJoin(Client& client) {
    // Check user parameters
    if (param_.size() == 0) {
        client.appendToSendBuffer(RPL_ERR_NEEDMOREPARAMS_461(serverHostname_g, client.getNickname(), "JOIN"));
        return;
    }
    // Leave all channels
    if (param_.size() == 1 && param_.at(0) == "0") {
        std::vector<std::string> myChannels = client.getMyChannels();
        for (std::string channelName : myChannels) {
            if (allChannels_.find(channelName) == allChannels_.end()) {
                LOG_ERROR(
                    "Command::actionJoin: Channel object not found using client's "
                    "channel list"
                    << client.getNickname() << " " << channelName)
                continue;
            }

            Channel& currentChannel = allChannels_.at(channelName);
            if (currentChannel.isMember(client) == false) {
                LOG_ERROR(
                    "Command::actionJoin: Client is not a member of the channel that "
                    "is in its channel list"
                    << client.getNickname() << " " << channelName)
                continue;
            }

            currentChannel.sendMessageToMembers(
                COM_MESSAGE(client.getNickname(), client.getUserName(), client.getHost(), "PART", client.getNickname()));
            currentChannel.partMember(client);
        }
        return;
    }

    // Get the channel names
    std::stringstream ssChan(param_.at(0));
    std::string rChannel;
    std::vector<std::pair<std::string, std::string>> rChannelKeyPairs;
    while (std::getline(ssChan, rChannel, ',')) {
        rChannelKeyPairs.push_back(std::make_pair(rChannel, ""));
    }

    // Get the channel keys
    if (param_.size() > 1) {
        std::stringstream ssKey(param_.at(1));
        std::string rKey;
        std::vector<std::pair<std::string, std::string>>::iterator itForKeys;
        for (itForKeys = rChannelKeyPairs.begin(); itForKeys != rChannelKeyPairs.end(); itForKeys++) {
            if (std::getline(ssKey, rKey, ',')) {
                itForKeys->second = rKey;
            }
        }
    }

    // Join the channels one by one
    std::vector<std::pair<std::string, std::string>>::iterator rit;
    for (rit = rChannelKeyPairs.begin(); rit != rChannelKeyPairs.end(); rit++) {
        std::string channelName = rit->first;
        std::string channelKey = rit->second;

        if (client.isMemberOfChannel(channelName)) {
            continue;
        }

        if (client.getMyChannels().size() >= MAX_JOIN_CHANNELS) {
            client.appendToSendBuffer(RPL_ERR_TOOMANYCHANNELS_405(serverHostname_g, channelName));
            break;
        }

        if (Channel::isChannelNameValid(channelName) == false) {
            client.appendToSendBuffer(RPL_ERR_NOSUCHCHANNEL_403(serverHostname_g, channelName));
            continue;
        }

        if (Channel::isChannelNameFree(channelName, allChannels_) == false) {
            Channel& existingChannel = allChannels_.at(channelName);
            if (channelKey != existingChannel.getKey()) {
                client.appendToSendBuffer(RPL_ERR_BADCHANNELKEY_475(serverHostname_g, client.getNickname(), channelName));
                continue;
            }
            if (existingChannel.getUserLimit() != CHANNEL_USER_LIMIT_DISABLED &&
                static_cast<long long>(existingChannel.getMemberCount()) >= existingChannel.getUserLimit()) {
                client.appendToSendBuffer(RPL_ERR_CHANNELISFULL_471(serverHostname_g, client.getNickname(), channelName));
                continue;
            }
            if (existingChannel.isInviteOnly() && existingChannel.isInvited(client) == false) {
                client.appendToSendBuffer(RPL_ERR_INVITEONLYCHAN_473(serverHostname_g, client.getNickname(), channelName));
                continue;
            }
            existingChannel.joinMember(client);
        } else {
            allChannels_.insert(std::make_pair(channelName, Channel(client, channelName, allChannels_)));
        }

        Channel& currentChannel = allChannels_.at(channelName);
        // Join message for the channel, and as a reply to the client
        currentChannel.sendMessageToMembers(COM_MESSAGE(client.getNickname(), client.getUserName(), client.getHost(), "JOIN", channelName));

        if (currentChannel.getTopic() != "") {
            client.appendToSendBuffer(
                RPL_TOPIC_332(serverHostname_g, client.getNickname(), channelName, currentChannel.getTopic()));
        } else {
            client.appendToSendBuffer(RPL_NOTOPIC_331(serverHostname_g, client.getNickname(), channelName));
        }

        client.appendToSendBuffer(
            RPL_NAMREPLY_353(serverHostname_g, client.getNickname(), CHANNEL_SYMBOL_PUBLIC, channelName, currentChannel.getNamesList()));
    }  // for (rit = rChannelKeyPairs.begin(); rit != rChannelKeyPairs.end(); rit++)
}

}  // namespace irc
