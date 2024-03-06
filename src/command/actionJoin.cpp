#include "Command.h"

namespace irc {

void Command::actionJoin(Client& client) {
  // Check user parameters
  if (param_.size() == 0) {
    client.appendToSendBuffer(
        RPL_ERR_NEEDMOREPARAMS_461(serverHostname_g, "JOIN"));
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

      Channel currentChannel = allChannels_.at(channelName);
      if (currentChannel.isMember(client) == false) {
        LOG_ERROR(
            "Command::actionJoin: Client is not a member of the channel that "
            "is in its channel list"
            << client.getNickname() << " " << channelName)
        continue;
      }

      currentChannel.sendMessageToMembers(
          COM_MESSAGE(client.getNickname(), client.getUserName(),
                      client.getHost(), "PART", client.getNickname()));
      currentChannel.partMember(client);
    }
    return;
  }

  // Get the channel names
  std::stringstream ssChan(param_.at(0));
  std::string rChannel;
  std::unordered_map<std::string, std::string> rChannelKeyPairs;
  while (std::getline(ssChan, rChannel, ',')) {
    rChannelKeyPairs[rChannel] = "";
  }

  // Get the channel keys
  if (param_.size() > 1) {
    std::stringstream ssKey(param_.at(1));
    std::string rKey;
    std::unordered_map<std::string, std::string>::iterator itForKeys;
    for (itForKeys = rChannelKeyPairs.begin();
         itForKeys != rChannelKeyPairs.end(); itForKeys++) {
      if (std::getline(ssKey, rKey, ',')) {
        itForKeys->second = rKey;
      }
    }
  }

  // Join the channels one by one
  std::unordered_map<std::string, std::string>::iterator rit;
  for (rit = rChannelKeyPairs.begin(); rit != rChannelKeyPairs.end(); rit++) {
    std::string channelName = rit->first;
    std::string channelKey = rit->second;
    if (client.getMyChannels().size() >= MAX_JOIN_CHANNELS) {
      client.appendToSendBuffer(
          RPL_ERR_TOOMANYCHANNELS_405(serverHostname_g, channelName));
      break;
    }
    // TODO: Check if the channel name is valid (regex) (ERR_NOSUCHCHANNEL)
    // if channel name is valid and does not exist, create the channel

    // else: do the things below
    // TODO: Check if there is a key for the channel and if it is valid (ERR_BADCHANNELKEY)
    // TODO: Check if the channel user limit is reached (ERR_CHANNELISFULL)
    // TODO: Check if the channel is invite-only, and invite is valid (ERR_INVITEONLYCHAN)
    // TODO: join the channel (how to access the channel object, and add it to the server's channel vector?)

    // TODO: send the required messages upon joining (JOIN to channel members, RPL_TOPIC, RPL_NAMREPLY including the joining client, RPL_ENDOFNAMES)
  }
}

}  // namespace irc
