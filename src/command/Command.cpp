#include "Command.h"

namespace irc {

std::map<std::string, std::function<void(Command*, Client&)>> Command::commands = {{"PING",
                                                                                    [](Command* cmd, Client& client) {
                                                                                        cmd->actionPing(client);
                                                                                    }},
                                                                                   {"CHANNEL",
                                                                                    [](Command* cmd, Client& client) {
                                                                                        cmd->actionChannel(client);
                                                                                    }},
                                                                                   {"PART",
                                                                                    [](Command* cmd, Client& client) {
                                                                                        cmd->actionPart(client);
                                                                                    }},
                                                                                   {"PASS",
                                                                                    [](Command* cmd, Client& client) {
                                                                                        cmd->actionPass(client);
                                                                                    }},
                                                                                   {"NICK",
                                                                                    [](Command* cmd, Client& client) {
                                                                                        cmd->actionNick(client);
                                                                                    }},
                                                                                   {"USER",
                                                                                    [](Command* cmd, Client& client) {
                                                                                        cmd->actionUser(client);
                                                                                    }},
                                                                                   {"QUIT",
                                                                                    [](Command* cmd, Client& client) {
                                                                                        cmd->actionQuit(client);
                                                                                    }},
                                                                                   {"PRIVMSG",
                                                                                    [](Command* cmd, Client& client) {
                                                                                        cmd->actionPrivmsg(client);
                                                                                    }},
                                                                                   {"JOIN",
                                                                                    [](Command* cmd, Client& client) {
                                                                                        cmd->actionJoin(client);
                                                                                    }},
                                                                                   {"TOPIC", [](Command* cmd, Client& client) {
                                                                                        cmd->actionTopic(client);
                                                                                    }}};

Command::Command(const Message& commandString, Client& client, std::map<int, Client>& allClients, std::string& password,
                 time_t& serverStartTime, std::map<std::string, Channel>& allChannels)
    : client_(client), allClients_(allClients), allChannels_(allChannels), pass_(password), serverStartTime_(serverStartTime) {
    numeric_ = 0;
    commandName_ = commandString.getCommand();
    prefix_ = commandString.getPrefix();
    param_ = commandString.getParameters();
    numeric_ = commandString.getNumeric();

    execute(client);
}

Command::~Command() {}

void Command::execute(Client& client) {
    if (client.isAuthenticated()) {
        auto it = commands.find(commandName_);
        if (it != commands.end()) {
            it->second(this, client);
        } else {
            client.appendToSendBuffer(RPL_ERR_UNKNOWNCOMMAND_421(serverHostname_g, commandName_));
            LOG_DEBUG("Command::execute: command not found: " << commandName_);
        }
        return;
    }

    if (commandName_ == "NICK" || commandName_ == "USER" || commandName_ == "PASS") {
        auto it = commands.find(commandName_);
        if (it != commands.end()) {
            it->second(this, client);
        } else {
            LOG_WARNING("Command::execute: NICK|USER|PASS command not found: " << commandName_);
        }
        return;
    }

    if (commandName_ == "CAP") {
        LOG_DEBUG("Command::execute: CAP command received, ignoring");
        return;
    }

    // If the client is sending a command before being authenticated
    client.appendToSendBuffer(RPL_ERR_NOTREGISTERED_451(serverHostname_g));
}

void Command::actionPing(Client& client) {
    client.appendToSendBuffer(RPL_MESSAGE("PONG " + serverHostname_g));
}

void Command::actionPass(Client& client) {
    if (client.isAuthenticated()) {
        client.appendToSendBuffer(RPL_ERR_ALREADYREGISTRED_462(serverHostname_g));
        return;
    }
    if (param_.size() == 0) {
        client.appendToSendBuffer(RPL_ERR_NEEDMOREPARAMS_461(serverHostname_g, "PASS"));
        return;
    }
    if (param_.at(0) != pass_) {
        LOG_DEBUG("Command::actionPass: password incorrect");
        client.appendToSendBuffer(ERR_MESSAGE("Password incorrect"));
        client.setWantDisconnect();
        return;
    }
    client.setPassword(pass_);
}

void Command::actionNick(Client& client) {
    bool isAlreadyAuthenticated = client.isAuthenticated();

    if (client.isGotPassword() == false) {
        client.appendToSendBuffer(ERR_MESSAGE("You must send a password first"));
        client.setWantDisconnect();
        return;
    }
    if (param_.size() == 0) {
        client.appendToSendBuffer(RPL_ERR_NONICKNAMEGIVEN_431(serverHostname_g));
        return;
    }

    if (!(isValidNickname(param_.at(0)))) {
        client.appendToSendBuffer(RPL_ERR_ERRONEUSNICKNAME_432(serverHostname_g, param_.at(0)));
        RPL_ERR_ERRONEUSNICKNAME_432(serverHostname_g, param_.at(0));
        return;
    }

    try {
        (void)findClientByNicknameOrThrow(param_.at(0));
        client.appendToSendBuffer(RPL_ERR_NICKNAMEINUSE_433(serverHostname_g, param_.at(0)));
        return;
    } catch (std::out_of_range& e) {
        ;  // The nickname is not in use
    }

    client.setNickname(param_.at(0));
    if (isAlreadyAuthenticated == false && client.isAuthenticated()) {
        sendAuthReplies_(client);
        return;  // Early return to avoid sending the NICK message for a just authenticated client IRCv3
    }

    // Send the NICK message to the client
    std::string nickMessage = COM_MESSAGE(client.getOldNickname(), client.getUserName(), client.getHost(), "NICK", client.getNickname());
    client.appendToSendBuffer(nickMessage);

    // Send the NICK message to all shared channels (do not resend to client)
    std::vector<std::string> channelNames = client.getMyChannels();
    if (channelNames.size() == 0) {
        return;
    }
    for (std::string channelName : channelNames) {
        Channel& channel = allChannels_.at(channelName);
        channel.sendMessageToMembersExcluding(nickMessage, client);
    }
}

void Command::actionUser(Client& client) {
    if (client.isGotPassword() == false) {
        client.appendToSendBuffer(ERR_MESSAGE("You must send a password first"));
        client.setWantDisconnect();
        return;
    }
    if (client.isAuthenticated()) {
        client.appendToSendBuffer(RPL_ERR_ALREADYREGISTRED_462(serverHostname_g));
        return;
    }
    if (param_.size() < 1) {
        client.appendToSendBuffer(RPL_ERR_NEEDMOREPARAMS_461(serverHostname_g, "USER"));
        return;
    }
    client.setUserName(param_.at(0));  // Note: We do not save the real name
    if (client.isAuthenticated()) {
        sendAuthReplies_(client);
    }
}

void Command::actionChannel(Client& client) {
    std::string response = ":" + serverHostname_g + " #newchannel " + serverHostname_g + " :" + client.getNickname();

    LOG_DEBUG(response);
}

void Command::actionPart(Client& client) {
    std::string partMessage = client.getNickname();
    if (param_.size() == 0) {
        client.appendToSendBuffer(RPL_ERR_NEEDMOREPARAMS_461(serverHostname_g, "PART"));
        return;
    }
    if (param_.size() >= 2) {
        partMessage = param_.at(1);
        if (partMessage.length() > 1 && partMessage[0] == ':') {
            partMessage = partMessage.substr(1);
        }
    }

    // Get the channel names
    std::stringstream ssChan(param_.at(0));
    std::string channelParseBuffer;
    std::vector<std::string> channelsToPart;
    while (std::getline(ssChan, channelParseBuffer, ',')) {
        channelsToPart.push_back(channelParseBuffer);
    }

    // Part the channels one by one
    for (std::string channelName : channelsToPart) {

        if (allChannels_.find(channelName) == allChannels_.end()) {
            client.appendToSendBuffer(RPL_ERR_NOSUCHCHANNEL_403(serverHostname_g, channelName));
            continue;
        }

        Channel& currentChannel = allChannels_.at(channelName);
        if (currentChannel.isMember(client) == false) {
            client.appendToSendBuffer(RPL_ERR_NOTONCHANNEL_442(serverHostname_g, channelName));
            continue;
        }

        currentChannel.sendMessageToMembers(COM_MESSAGE(client.getNickname(), client.getUserName(), client.getHost(), "PART", partMessage));
        currentChannel.partMember(client);
    }
}

void Command::actionTopic(Client& client) {
    if (param_.size() == 0) {
        client.appendToSendBuffer(RPL_ERR_NEEDMOREPARAMS_461(serverHostname_g, "TOPIC"));
        return;
    }
    std::vector<std::string> myChannels = client.getMyChannels();
    auto it = allChannels_.find(param_.at(0));
    if (it == allChannels_.end()) {
        client.appendToSendBuffer(RPL_ERR_NOSUCHCHANNEL_403(serverHostname_g, param_.at(0)));
        return;
    }
    if (param_.size() == 1) {
        if (it != allChannels_.end()) {
            if (it->second.getTopic().empty()) {
                client.appendToSendBuffer(RPL_NOTOPIC_331(serverHostname_g, param_.at(0)));
            }
            client.appendToSendBuffer(
                RPL_TOPIC_332(client.getNickname(), client.getUserName(), serverHostname_g, param_.at(0), it->second.getTopic()));
            return;
        }
    }
    if (param_.size() > 1) {
        if (it->second.isMember(client) == false) {
            LOG_ERROR("Command::actionTopic: the Client : " << client.getNickname() << "not a member of the channel"
                                                            << " " << param_.at(0))
            client.appendToSendBuffer(RPL_ERR_NOTONCHANNEL_442(serverHostname_g, param_.at(0)));
            return;
        }
        std::string topic = param_.back();
        if (topic[0] == ':') {
            it->second.setTopic(param_.back());
            LOG_DEBUG(topic);
            it->second.sendMessageToMembers(
                RPL_TOPIC_332(client.getNickname(), client.getUserName(), serverHostname_g, param_.at(0), it->second.getTopic()));
            return;
        }
        //client.appendToSendBuffer(RPL_ERR_NEEDMOREPARAMS_461(serverHostname_g, "TOPIC"));
        return;
    }
}

void Command::actionMode(Client& client) {
    std::string response = ":" + serverHostname_g + " #newchannel " + serverHostname_g + " :" + client.getNickname();

    // here we need to put the four parts
    LOG_DEBUG(response);
}

void Command::actionKick(Client& client) {
    std::string response = ":" + serverHostname_g + " #newchannel " + serverHostname_g + " :" + client.getNickname();

    LOG_DEBUG(response);
}

/**
 * @brief A client session is terminated with a quit message.
 * The server acknowledges this by sending an ERROR message to the client
 * and relays the QUIT message to all other clients on shared channels.
 * 
 * @param client The client that is quitting
 * @example QUIT :Gone to have lunch                        // Message from Client
 *          ERROR :Bye, see you soon!                       // Response from Server
 *          :nick!user@servername QUIT :Gone to have lunch  // Relayed to other Clients on shared channels
 */
void Command::actionQuit(Client& client) {
    std::string reason;
    if (param_.size() != 0) {
        if (param_.at(0).length() > 0 && param_.at(0).front() == ':') {
            reason = param_.at(0).substr(1);
        }
        reason = std::string("Quit: ") + std::string(reason);
    } else {
        reason = "Quit: ";
    }
    if (client.getDisconnectReason().empty()) {
        client.setDisconnectReason(reason);
    }
    if (client.getDisconnectErrorReason().empty()) {
        client.setDisconnectErrorReason(reason);
    }
    client.setWantDisconnect();
}

Client& Command::findClientByNicknameOrThrow(const std::string& nickname) {
    std::string lowerNickname = nickname;  //here we just put everything in lowercase
    std::transform(lowerNickname.begin(), lowerNickname.end(), lowerNickname.begin(), ::tolower);
    std::replace(lowerNickname.begin(), lowerNickname.end(), '{', '[');
    std::replace(lowerNickname.begin(), lowerNickname.end(), '}', ']');
    std::replace(lowerNickname.begin(), lowerNickname.end(), '|', '\\');
    std::replace(lowerNickname.begin(), lowerNickname.end(), '^', '~');

    for (std::map<int, Client>::const_iterator it = allClients_.begin(); it != allClients_.end(); ++it) {
        std::string clientNickname = it->second.getNickname();  //here we just put everything in lowercase
        std::transform(clientNickname.begin(), clientNickname.end(), clientNickname.begin(), ::tolower);
        std::replace(clientNickname.begin(), clientNickname.end(), '{', '[');
        std::replace(clientNickname.begin(), clientNickname.end(), '}', ']');
        std::replace(clientNickname.begin(), clientNickname.end(), '|', '\\');
        std::replace(clientNickname.begin(), clientNickname.end(), '^', '~');
        if (clientNickname == lowerNickname) {
            LOG_DEBUG("CMD::findClientByNickname: Found client with the same nickname: " << nickname);
            return allClients_.at(it->first);
        }
    }

    LOG_DEBUG(std::string("Command::findClientByNicknameOrThrow: ") + nickname + " not found");
    throw std::out_of_range(std::string("Command::findClientByNicknameOrThrow: ") + nickname + " not found");
}

bool Command::isValidNickname(std::string& nickname) {
    std::regex pattern(R"(^[a-zA-Z\[\]\\`_^{|}])");
    std::regex pattern1(R"(^[a-zA-Z0-9\[\]\\`_^{|}-]*$)");
    if (!std::regex_match(nickname.substr(0, 1), pattern)) {
        return false;
    }
    if (!std::regex_match(nickname, pattern1)) {
        return false;
    }
    if (nickname.size() > 9) {
        nickname = nickname.substr(0, 9);
        LOG_DEBUG("Command::isValidNickname: nick was too long, shortened to: " << nickname);
    }
    return true;
}

void Command::sendAuthReplies_(Client& client) {
    std::string clientNick = client.getNickname();
    client.appendToSendBuffer(RPL_WELCOME_001(serverHostname_g, clientNick, client.getUserName(), client.getHost()));
    client.appendToSendBuffer(RPL_YOURHOST_002(serverHostname_g, clientNick, IRC_SERVER_VERSION));
    std::string time = std::string(ctime(&serverStartTime_));
    time.pop_back();  // Remove the newline character
    client.appendToSendBuffer(RPL_CREATED_003(serverHostname_g, clientNick, time));
    client.appendToSendBuffer(
        RPL_MYINFO_004(serverHostname_g, clientNick, IRC_SERVER_VERSION, SUPPORTED_USER_MODES, SUPPORTED_CHANNEL_MODES));
}
}  // namespace irc
