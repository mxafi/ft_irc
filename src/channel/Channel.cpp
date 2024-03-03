#include "Channel.h"

namespace irc {

Channel::~Channel() {}

Channel::Channel(Client& creatorClient, const std::string& name)
    : name_(name),
      isInviteOnly_(false),
      isTopicProtected_(false),
      userLimit_(CHANNEL_USER_LIMIT_DISABLED) {
  if (name == "taken") {  // TODO: Implement check if channel name is valid
    throw std::invalid_argument("Channel name is invalid or taken");
  }
  members_.push_back(&creatorClient);
  operators_.push_back(&creatorClient);
  creatorClient.recordChannel(*this);
}

std::string Channel::getName() const {
  return name_;
}

void Channel::sendMessageToMembers(const std::string& message) {
  for (Client* member : members_) {
    if (member == nullptr) {
      LOG_ERROR("Channel::sendMessageToMember: member is null, skipping");
      continue;
    }
    member->appendToSendBuffer(message);
  }
}

void Channel::setTopic(const std::string& topic) {
  topic_ = topic;
}

std::string Channel::getTopic() const {
  return topic_;
}

void Channel::setKey(const std::string& key) {
  key_ = key;
}

std::string Channel::getKey() const {
  return key_;
}

void Channel::setInviteOnly(bool isInviteOnly) {
  isInviteOnly_ = isInviteOnly;
}

bool Channel::isInviteOnly() const {
  return isInviteOnly_;
}

void Channel::setTopicProtected(bool isTopicProtected) {
  isTopicProtected_ = isTopicProtected;
}

bool Channel::isTopicProtected() const {
  return isTopicProtected_;
}

void Channel::setUserLimit(int userLimit) {
  userLimit_ = userLimit;
}

int Channel::getUserLimit() const {
  return userLimit_;
}

void Channel::joinMember(Client& client) {
  if (isInviteOnly_ && !isInvited(client)) {
    LOG_WARNING("Channel::joinMember: client is not invited, not joining")
    return;
  }
  if (userLimit_ != CHANNEL_USER_LIMIT_DISABLED &&
      static_cast<int>(members_.size()) >= userLimit_) {
    LOG_WARNING("Channel::joinMember: user limit reached, not joining")
    return;
  }
  members_.push_back(&client);
  client.recordChannel(*this);
}

void Channel::partMember(Client& client) {
  for (auto it = members_.begin(); it != members_.end(); it++) {
    if (*it == &client) {
      client.removeChannel(*this);
      if (isOperator(client)) {
        setOperatorStatus(client, false);
      }
      members_.erase(it);
      if (members_.empty()) {
        LOG_DEBUG(
            "Channel::partMember: no members left, the channel should be "
            "deleted: "
            << name_);
      }
      return;
    }
  }
  LOG_WARNING("Channel::partMember: client is not a member, not parting");
}

bool Channel::isMember(Client& client) {
  for (Client* member : members_) {
    if (member == &client) {
      return true;
    }
  }
  return false;
}

void Channel::setOperatorStatus(Client& client, bool setOperatorStatusTo) {
  if (!isMember(client)) {
    LOG_WARNING(
        "Channel::setOperatorStatus: client is not a member, not setting "
        "operator status")
    return;
  }
  if (setOperatorStatusTo && !isOperator(client)) {
    operators_.push_back(&client);
    LOG_DEBUG("Channel::setOperatorStatus: operator status set to "
              << client.getNickname() << " in channel " << name_);
  } else {
    for (auto it = operators_.begin(); it != operators_.end(); it++) {
      if (*it == &client) {
        operators_.erase(it);
        LOG_DEBUG("Channel::setOperatorStatus: operator status removed from "
                  << client.getNickname() << " in channel " << name_);
        return;
      }
    }
  }
}

bool Channel::isOperator(Client& client) {
  for (Client* operator_ : operators_) {
    if (operator_ == &client) {
      return true;
    }
  }
  return false;
}

void Channel::invite(Client& client) {
  if (isInvited(client)) {
    LOG_WARNING("Channel::invite: client is already invited, not inviting");
    return;
  }
  invited_.push_back(&client);
}

void Channel::uninvite(Client& client) {
  for (auto it = invited_.begin(); it != invited_.end(); it++) {
    if (*it == &client) {
      invited_.erase(it);
      return;
    }
  }
  LOG_WARNING("Channel::uninvite: client is not invited, not uninviting");
}

bool Channel::isInvited(Client& client) {
  for (Client* invited : invited_) {
    if (invited == &client) {
      return true;
    }
  }
  return false;
}

}  // namespace irc
