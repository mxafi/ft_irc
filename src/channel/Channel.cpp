#include "Channel.h"

namespace irc {

// TODO: Maybe unrecord the channel from all clients, and erase from allChannels_?
Channel::~Channel() {}

Channel::Channel(Client& creatorClient, const std::string& name, std::map<std::string, Channel>& allChannels)
    : name_(name), isInviteOnly_(false), isTopicProtected_(false), userLimit_(CHANNEL_USER_LIMIT_DISABLED), allChannels_(allChannels) {

    if (!isChannelNameValid(name) || !isChannelNameFree(name, allChannels_)) {
        throw std::invalid_argument("Channel name is invalid or taken");
    }
    members_.push_back(&creatorClient);
    operators_.push_back(&creatorClient);
    creatorClient.recordMyChannel(name_);
    LOG_DEBUG("Channel::Channel: channel " << name_ << " created by " << creatorClient.getNickname());
}

std::string Channel::getName() const {
    return name_;
}

void Channel::sendMessageToMembers(const std::string& message) {
    for (Client* member : members_) {
        member->appendToSendBuffer(message);
    }
}

void Channel::sendMessageToMembersExcluding(const std::string& message, const Client& excludedClient) {
    for (Client* member : members_) {
        if (member->getFd() != excludedClient.getFd()) {
            member->appendToSendBuffer(message);
        }
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
    if (userLimit_ != CHANNEL_USER_LIMIT_DISABLED && static_cast<int>(members_.size()) >= userLimit_) {
        LOG_WARNING("Channel::joinMember: user limit reached, not joining")
        return;
    }
    members_.push_back(&client);
    client.recordMyChannel(name_);
    LOG_DEBUG("Channel::joinMember: client " << client.getNickname() << " joined channel " << name_);
}

/**
 * @brief Remove a member from the channel members,
 * operators and unrecord the channel from the client
 * 
 * @param client The client to be removed
 * @return int Number of members left in the channel, or CHANNEL_PART_FAILURE
 * if the client is not a member
 */
int Channel::partMember(Client& client) {
    int clientFd = client.getFd();
    for (auto it = members_.begin(); it != members_.end(); it++) {
        if ((*it)->getFd() == clientFd) {
            client.unrecordMyChannel(name_);
            if (isOperator(client)) {
                LOG_DEBUG("Channel::partMember: client " << client.getNickname() << " was an operator, removing");
                setOperatorStatus(client, false);
            }
            members_.erase(it);
            if (operators_.empty()) {
                LOG_DEBUG("Channel::partMember: no operators left, a new operator should be assigned: " << name_);
                if (!members_.empty()) {
                    setOperatorStatus(*members_.front(), true);
                }
            }
            LOG_DEBUG("Channel::partMember: client " << client.getNickname() << " parted from channel " << name_);
            return static_cast<int>(members_.size());
        }
        if (members_.empty()) {
            LOG_DEBUG(
                "Channel::partMember: no members left, the channel should be "
                "deleted: "
                << name_);
            // TODO: Delete the channel in this or the caller function
            return static_cast<int>(members_.size());
        }
    }
    LOG_WARNING("Channel::partMember: client is not a member, not parting nick " << name_);
    return CHANNEL_PART_FAILURE;
}

bool Channel::isMember(Client& client) {
    int clientFd = client.getFd();
    for (Client* member : members_) {
        if (member->getFd() == clientFd) {
            return true;
        }
    }
    return false;
}

void Channel::setOperatorStatus(Client& client, bool setOperatorStatusTo) {
    int clientFd = client.getFd();

    // If the client is not a member, do not set operator status
    if (!isMember(client)) {
        LOG_WARNING("Channel::setOperatorStatus: client is not a member, not setting operator status");
        return;
    }

    bool isOp = isOperator(client);
    // Setting operator status to true
    if (setOperatorStatusTo) {
        // Client is an operator, do not add to operators_
        if (isOp) {
            LOG_WARNING("Channel::setOperatorStatus: client is already an operator, not setting operator status for "
                        << client.getNickname() << " in " << name_);
            return;
        }

        // Client is not an operator, add to operators_
        operators_.push_back(&client);
        LOG_DEBUG("Channel::setOperatorStatus: operator status added to " << client.getNickname() << " in channel " << name_);
        return;
    }

    // Setting operator status to false
    // Client is not not an operator, do not remove from operators_
    if (!isOp) {
        LOG_WARNING("Channel::setOperatorStatus: client is not an operator, not removing operator status for " << client.getNickname()
                                                                                                               << " in " << name_);
        return;
    }

    // Client is an operator, remove from operators_
    for (auto it = operators_.begin(); it != operators_.end(); it++) {
        if ((*it)->getFd() == clientFd) {
            operators_.erase(it);
            LOG_DEBUG("Channel::setOperatorStatus: operator status removed from " << client.getNickname() << " in channel " << name_);
            return;
        }
    }

    LOG_ERROR("Channel::setOperatorStatus: client is an operator, but not found in operators_, nick: " << client.getNickname() << " in "
                                                                                                       << name_);
}

bool Channel::isOperator(Client& client) {
    int clientFd = client.getFd();
    for (Client* operator_ : operators_) {
        if (operator_->getFd() == clientFd) {
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
    LOG_DEBUG("Channel::invite: inviting client " << client.getNickname() << " to channel " << name_);
    invited_.push_back(&client);
}

void Channel::uninvite(Client& client) {
    int clientFd = client.getFd();
    for (auto it = invited_.begin(); it != invited_.end(); it++) {
        if ((*it)->getFd() == clientFd) {
            LOG_DEBUG("Channel::uninvite: uninviting client " << client.getNickname() << " from channel " << name_);
            invited_.erase(it);
            return;
        }
    }
    LOG_WARNING("Channel::uninvite: client is not invited, not uninviting");
}

bool Channel::isInvited(Client& client) {
    int clientFd = client.getFd();
    for (Client* invited : invited_) {
        if (invited->getFd() == clientFd) {
            return true;
        }
    }
    return false;
}

bool Channel::isChannelNameValid(const std::string& name) {
    std::regex pattern(R"(^[&#][a-zA-Z0-9_-]+$)", std::regex::icase);

    if (name.length() < MIN_CHANNELNAME_LENGTH || name.length() > MAX_CHANNELNAME_LENGTH) {
        LOG_DEBUG("Channel::isChannelNameValid: name length is invalid");
        return false;
    }
    if (std::regex_match(name, pattern)) {
        return true;
    }
    LOG_DEBUG("Channel::isChannelNameValid: name does not match pattern");
    return false;
}

bool Channel::isChannelNameFree(const std::string& name, std::map<std::string, Channel>& allChannels) {
    std::string lowercaseName = name;
    std::transform(lowercaseName.begin(), lowercaseName.end(), lowercaseName.begin(), ::tolower);

    for (auto it = allChannels.begin(); it != allChannels.end(); it++) {
        if (it->second.getName() == lowercaseName) {
            return false;
        }
    }
    return true;
}

unsigned long Channel::getMemberCount() const {
    return members_.size();
}

std::vector<Client*>& Channel::getMembers() {
    return members_;
}

std::string Channel::getNamesList() {
    std::string namesList;
    for (Client* member : members_) {
        if (isOperator(*member)) {
            namesList.append(CHANNEL_OPERATOR_SYMBOL);
        }
        namesList.append(member->getNickname() + " ");
    }
    // get rid of the trailing space
    namesList.pop_back();
    return namesList;
}

}  // namespace irc
