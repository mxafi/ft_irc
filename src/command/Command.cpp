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
                                                                                   {"TOPIC",
                                                                                    [](Command* cmd, Client& client) {
                                                                                        cmd->actionTopic(client);
                                                                                    }},
                                                                                   {"KICK",
                                                                                    [](Command* cmd, Client& client) {
                                                                                        cmd->actionKick(client);
                                                                                    }},
                                                                                   {"MODE", [](Command* cmd, Client& client) {
                                                                                        cmd->actionMode(client);
                                                                                    }},
                                                                                   {"INVITE", [](Command* cmd, Client& client) {
                                                                                        cmd->actionInvite(client);
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
        client.appendToSendBuffer(RPL_ERR_ALREADYREGISTRED_462(serverHostname_g, client.getNickname()));
        return;
    }
    if (param_.size() == 0) {
        client.appendToSendBuffer(RPL_ERR_NEEDMOREPARAMS_461(serverHostname_g, client.getNickname(), "PASS"));
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

/**
 * @brief Handles the NICK command, changing a client's nickname.
 *
 * This function checks if the client has sent a valid authentication password, if a nickname was provided,
 * and if the nickname is valid. It also checks if the nickname is already in use.
 * If all conditions are met, it changes the client's nickname and sends the appropriate
 * messages to the client and other clients in shared channels.
 *
 * @param client The client whose nickname is to be changed.
 */
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
        client.appendToSendBuffer(RPL_ERR_ALREADYREGISTRED_462(serverHostname_g, client.getNickname()));
        return;
    }
    if (param_.size() < 1) {
        client.appendToSendBuffer(RPL_ERR_NEEDMOREPARAMS_461(serverHostname_g, client.getNickname(), "USER"));
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

void Command::actionInvite(Client& client) {
    if (param_.size() < 2) {
        client.appendToSendBuffer(RPL_ERR_NEEDMOREPARAMS_461(serverHostname_g, "INVITE"));
        return;
    }
    std::string nickname = param_.at(0);
    std::string channelName = param_.at(1);
    try {
        (void)findClientByNicknameOrThrow(nickname);
    } catch (std::out_of_range& e) {
        client.appendToSendBuffer(RPL_ERR_NOSUCHNICK_401(serverHostname_g, nickname));
        return;
    }
    //it means that the nick exits
    Client& invitee = findClientByNicknameOrThrow(nickname);
    auto it = allChannels_.find(channelName);
    if (it != allChannels_.end()) {
        Channel& channel = allChannels_.at(channelName);
        if (!channel.isMember(client)) {
            client.appendToSendBuffer(RPL_ERR_NOTONCHANNEL_442(serverHostname_g, channelName));
            return;
        }
        if (channel.isInviteOnly() == true) {
            if (!channel.isOperator(client)) {
                client.appendToSendBuffer(RPL_ERR_CHANOPRIVSNEEDED_482(serverHostname_g, channelName));
                return;
            }
        }
        if (channel.isMember(invitee)) {
            client.appendToSendBuffer(RPL_ERR_USERONCHANNEL_443(serverHostname_g, nickname, channelName));
            return;
        }
        if (!channel.isInvited(invitee)) {
            channel.invite(invitee);
        }
    }
    client.appendToSendBuffer(RPL_INVITING_341(serverHostname_g, client.getNickname(), nickname, channelName));
    invitee.appendToSendBuffer(INVITE(serverHostname_g, client.getNickname(), client.getUserName(), nickname, channelName));
}

void Command::actionPart(Client& client) {
    std::string partMessage = client.getNickname();
    if (param_.size() == 0) {
        client.appendToSendBuffer(RPL_ERR_NEEDMOREPARAMS_461(serverHostname_g, client.getNickname(), "PART"));
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
            client.appendToSendBuffer(RPL_ERR_NOTONCHANNEL_442(serverHostname_g, client.getNickname(), channelName));
            continue;
        }

        currentChannel.sendMessageToMembers(COM_MESSAGE(client.getNickname(), client.getUserName(), client.getHost(), "PART",
                                                        currentChannel.getName() + " :" + partMessage));
        currentChannel.partMember(client);
    }
}

// TOPIC #channel :desiredTopic
void Command::actionTopic(Client& client) {
    std::string topicParam;
    // Check parameters
    // 0: reply ERR_NEEDMOREPARAMS
    if (param_.size() == 0) {
        client.appendToSendBuffer(RPL_ERR_NEEDMOREPARAMS_461(serverHostname_g, client.getNickname(), "TOPIC"));
        return;
    }
    if (param_.size() >= 2) {
        // Case 1: p1 has :topic --> topicParam = "topic"
        // Case 2: p1 has topic --> topicParam = "topic"
        // Case 3: p1 has : --> topicParam = ""
        if (param_.at(1).front() == ':') {
            if (param_.at(1).length() > 1) {
                topicParam = param_.at(1).substr(1);
            } else {
                topicParam = "";  // indicates the desire to remove the topic
            }
        } else {
            topicParam = param_.at(1);
        }
    }

    // validate client and channel for getting the topic
    std::vector<std::string> clientChannels = client.getMyChannels();
    bool isMember = false;
    for (std::string clientChannel : clientChannels) {
        if (clientChannel == param_.at(0)) {
            isMember = true;
            break;
        }
    }
    if (isMember == false) {
        try {
            Channel& channel = allChannels_.at(param_.at(0));
            (void)channel;  // suppress warning about unused variable
            client.appendToSendBuffer(RPL_ERR_NOTONCHANNEL_442(serverHostname_g, client.getNickname(), param_.at(0)));
            return;
        } catch (std::out_of_range& e) {
            client.appendToSendBuffer(RPL_ERR_NOSUCHCHANNEL_403(serverHostname_g, param_.at(0)));
            return;
        }
    }

    // at this point, the client is a member of the channel and it exists
    Channel& channel = allChannels_.at(param_.at(0));

    // get the topic
    if (param_.size() == 1) {
        if (channel.getTopic().empty()) {
            client.appendToSendBuffer(RPL_NOTOPIC_331(serverHostname_g, client.getNickname(), param_.at(0)));
            return;
        }
        client.appendToSendBuffer(RPL_TOPIC_332(serverHostname_g, client.getNickname(), param_.at(0), channel.getTopic()));
        return;
    }

    // at this point, the client wants to set the topic

    // check if the client has the right to set the topic
    if (channel.isTopicProtected() && !channel.isOperator(client)) {
        client.appendToSendBuffer(RPL_ERR_CHANOPRIVSNEEDED_482(serverHostname_g, client.getNickname(), param_.at(0)));
        return;
    }

    // set the topic
    channel.setTopic(topicParam);

    // send the topic to all members, should the client be exluded and rpl_topic_332 be sent? TODO
    channel.sendMessageToMembers(
        COM_MESSAGE(client.getNickname(), client.getUserName(), client.getHost(), "TOPIC", channel.getName() + " :" + topicParam));
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

/**
 * @brief Finds a client by their nickname, throwing an exception if not found.
 *
 * This function searches for a client with the specified nickname. It converts the
 * nickname and the client nicknames to lowercase and replaces certain characters to
 * match the IRC specification. If the client is found, it returns a reference to the
 * client. If not, it throws an std::out_of_range exception.
 *
 * @param  The nickname to search for.
 * @return A reference to the client with the matching nickname.
 * @throws std::out_of_range if the client is not found.
 */
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

/**
 *   @brief Checks if a given nickname is valid according to RFC2812.
 *
 *   nickname   =  ( letter / special ) *8( letter / digit / special / "-" )
 *   where,
 *       special = %x5B-60 / %x7B-7D 
 *       ; "[", "]", "\", "‘", "_", "^", "{", "|", "}"
 *   
 *   Exception is made for spaces which we accept as DALnet does, i.e. using the 
 *   space as a delimiter setting the nick to the first delimited word.
 *   
 *   A nickname can be of maximum 9 characters long
 *    
 *   @param  The nickname to validate.
 *   @return true if the nickname is valid, false otherwise.
 */
bool Command::isValidNickname(std::string& nickname) {
    std::regex pattern(R"(^[a-zA-Z\[\]\\`_^{|}])");
    std::regex pattern1(R"(^[a-zA-Z0-9\[\]\\`_^{|}-]*$)");
    if (!std::regex_match(nickname.substr(0, 1), pattern)) {
        return false;
    }
    if (!std::regex_match(nickname, pattern1)) {
        return false;
    }
    if (nickname.size() > NICK_MAX_LENGTH_RFC2812) {
        nickname = nickname.substr(0, NICK_MAX_LENGTH_RFC2812);
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
