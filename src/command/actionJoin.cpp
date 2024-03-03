#include "Command.h"

namespace irc {

void Command::actionJoin(Client& client) {
  if (param_.size() == 0) {
    client.appendToSendBuffer(
        RPL_ERR_NEEDMOREPARAMS_461(serverHostname_g, "JOIN"));
    return;
  }
  if (param_.size() == 1 && param_.at(0) == "0") {
    // Leave all channels the client is in using PART
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
    if (client.getChannels().size() >= MAX_JOIN_CHANNELS) {
      client.appendToSendBuffer(
          RPL_ERR_TOOMANYCHANNELS_405(serverHostname_g, channelName));
      break;
    }
    // Check if the channel name is valid (regex) (ERR_NOSUCHCHANNEL) remember to create if valid and not reply with error
    // Check if there is a key for the channel and if it is valid (ERR_BADCHANNELKEY)
    // Check if the channel user limit is reached (ERR_CHANNELISFULL)
    // Check if the channel is invite-only, and invite is valid (ERR_INVITEONLYCHAN)
    // join the channel
    // send the required messages upon joining (JOIN to channel members, RPL_TOPIC, RPL_NAMREPLY including the joining client, RPL_ENDOFNAMES)
  }
}

}  // namespace irc
