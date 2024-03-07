#include "Command.h"

namespace irc {

std::map<std::string, std::function<void(Command*, Client&)>>
    Command::commands = {{"PING",
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
                         {"JOIN", [](Command* cmd, Client& client) {
                            cmd->actionJoin(client);
                          }}};

Command::Command(const Message& commandString, Client& client,
                 std::map<int, Client>& myClients, std::string& password,
                 time_t& serverStartTime,
                 std::map<std::string, Channel>& allChannels)
    : client_(client),
      allClients_(myClients),
      allChannels_(allChannels),
      pass_(password),
      serverStartTime_(serverStartTime) {
  numeric_ = 0;
  parseCommand(commandString, client);
}

Command::~Command() {}

void Command::execute(Client& client) {
  if (client.isAuthenticated()) {
    auto it = commands.find(commandName_);
    if (it != commands.end()) {
      it->second(this, client);
    } else {
      client.appendToSendBuffer(
          RPL_ERR_UNKNOWNCOMMAND_421(serverHostname_g, commandName_));
      LOG_DEBUG("Command::execute: command not found: " << commandName_);
    }
    return;
  }

  if (commandName_ == "NICK" || commandName_ == "USER" ||
      commandName_ == "PASS") {
    auto it = commands.find(commandName_);
    if (it != commands.end()) {
      it->second(this, client);
    } else {
      LOG_WARNING("Command::execute: NICK|USER|PASS command not found: "
                  << commandName_);
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

void Command::parseCommand(const Message& commandString, Client& client) {
  commandName_ = commandString.getCommand();
  prefix_ = commandString.getPrefix();
  param_ = commandString.getParameters();
  numeric_ = commandString.getNumeric();
  execute(client);
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
    client.appendToSendBuffer(
        RPL_ERR_NEEDMOREPARAMS_461(serverHostname_g, "PASS"));
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

  // Check if the nickname is valid, below are rules and info about it
  // nickname   =  ( letter / special ) *8( letter / digit / special / "-" )
  // letter     =  %x41-5A / %x61-7A       ; A-Z / a-z
  // digit      =  %x30-39                 ; 0-9
  // special    =  %x5B-60 / %x7B-7D       ; "[", "]", "\", "`", "_", "^", "{", "|", "}"
  // Numerics: ERR_ERRONEUSNICKNAME  and also cut it for 9 ccharacters
  if (!(isValidNickname(param_.at(0)))) {
    client.appendToSendBuffer(
        RPL_ERR_ERRONEUSNICKNAME_432(serverHostname_g, param_.at(0)));
    RPL_ERR_ERRONEUSNICKNAME_432(serverHostname_g, param_.at(0));
    return;
  }

  // Check if the nickname is already in use
  // Because of IRC's Scandinavian origin, the characters {}|^ are
  // considered to be the lower case equivalents of the characters []\~,
  // respectively. This is a critical issue when determining the
  // equivalence of two nicknames or channel names.
  // When evaluating nickname equivalence, let's convert all characters to lower case.
  // Numerics: ERR_NICKNAMEINUSE
  if (findClientByNickname(param_.at(0))) {
    client.appendToSendBuffer(
        RPL_ERR_NICKNAMEINUSE_433(serverHostname_g, param_.at(0)));
    return;
  }

  client.setNickname(param_.at(0));
  if (isAlreadyAuthenticated == false && client.isAuthenticated()) {
    sendAuthReplies_(client);
  }

  // TODO: Send a NICK message to all channels the client is in, advertising the new nickname
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
    client.appendToSendBuffer(
        RPL_ERR_NEEDMOREPARAMS_461(serverHostname_g, "USER"));
    return;
  }
  client.setUserName(param_.at(0));  // Note: We do not save the real name
  if (client.isAuthenticated()) {
    sendAuthReplies_(client);
  }
}

void Command::actionChannel(Client& client) {
  std::string response = ":" + serverHostname_g + " #newchannel " +
                         serverHostname_g + " :" + client.getNickname();

  LOG_DEBUG(response);
}

void Command::actionPart(Client& client) {
  std::string partMessage = client.getNickname();
  if (param_.size() == 0) {
    client.appendToSendBuffer(
        RPL_ERR_NEEDMOREPARAMS_461(serverHostname_g, "PART"));
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
      client.appendToSendBuffer(
          RPL_ERR_NOSUCHCHANNEL_403(serverHostname_g, channelName));
      continue;
    }

    Channel currentChannel = allChannels_.at(channelName);
    if (currentChannel.isMember(client) == false) {
      client.appendToSendBuffer(
          RPL_ERR_NOTONCHANNEL_442(serverHostname_g, channelName));
      continue;
    }

    currentChannel.sendMessageToMembers(
        COM_MESSAGE(client.getNickname(), client.getUserName(),
                    client.getHost(), "PART", partMessage));
    currentChannel.partMember(client);
  }
}

void Command::actionMode(Client& client) {
  std::string response = ":" + serverHostname_g + " #newchannel " +
                         serverHostname_g + " :" + client.getNickname();

  // here we need to put the four parts
  LOG_DEBUG(response);
}

void Command::actionKick(Client& client) {
  std::string response = ":" + serverHostname_g + " #newchannel " +
                         serverHostname_g + " :" + client.getNickname();

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
  // Find out the reason for quitting
  std::string quitReason;
  if (param_.size() != 0) {
    quitReason = param_.at(0);
  } else {
    quitReason = "Client quit";
  }
  if (quitReason.length() > 1 && quitReason[0] == ':') {
    quitReason = quitReason.substr(1);
  }
  if (client.getDisconnectReason().empty()) {
    client.setDisconnectReason(quitReason);
  }

  // Send the QUIT message to all shared channels with the reason
  std::vector<std::string> channelNames = client.getMyChannels();
  for (std::string channelName : channelNames) {
    Channel& channel = allChannels_.at(channelName);
    channel.sendMessageToMembers(
        COM_MESSAGE(client.getNickname(), client.getUserName(),
                    client.getHost(), "QUIT", quitReason));
  }

  // Send the ERROR message to the client and disconnect
  client.appendToSendBuffer(ERR_MESSAGE("Bye, see you soon!"));
  client.setWantDisconnect();
}

void Command::actionPrivmsg(Client& client) {

  std::string replyPrivmsg = "here you put \r\n";

  client.appendToSendBuffer(replyPrivmsg);
}

bool Command::findClientByNickname(const std::string& nickname) {
  std::string lowerNickname =
      nickname;  //here we just put everything in lowercase
  std::transform(lowerNickname.begin(), lowerNickname.end(),
                 lowerNickname.begin(), ::tolower);
  std::replace(lowerNickname.begin(), lowerNickname.end(), '{', '[');
  std::replace(lowerNickname.begin(), lowerNickname.end(), '}', ']');
  std::replace(lowerNickname.begin(), lowerNickname.end(), '|', '\\');
  std::replace(lowerNickname.begin(), lowerNickname.end(), '^', '~');

  for (std::map<int, Client>::const_iterator it = allClients_.begin();
       it != allClients_.end(); ++it) {
    std::string clientNickname =
        it->second.getNickname();  //here we just put everything in lowercase
    std::transform(clientNickname.begin(), clientNickname.end(),
                   clientNickname.begin(), ::tolower);
    std::replace(clientNickname.begin(), clientNickname.end(), '{', '[');
    std::replace(clientNickname.begin(), clientNickname.end(), '}', ']');
    std::replace(clientNickname.begin(), clientNickname.end(), '|', '\\');
    std::replace(clientNickname.begin(), clientNickname.end(), '^', '~');
    if (clientNickname == lowerNickname) {
      LOG_DEBUG(
          "Command::findClientByNickname: found client with the same nickname: "
          << nickname);
      return true;
    }
  }
  return false;
}

bool Command::isValidNickname(std::string& nickname) {
  std::regex pattern(R"(^[a-zA-Z\[\]\\`_^{|}])");
  std::regex pattern1(R"(^[a-zA-Z0-9\[\]\\,`_^{|}-]*$)");
  if (!std::regex_match(nickname.substr(0, 1), pattern)) {
    return false;
  }
  if (!std::regex_match(nickname, pattern1)) {
    return false;
  }
  if (nickname.size() > 9) {
    nickname = nickname.substr(0, 9);
    LOG_DEBUG("Command::isValidNickname: nick was too long, shortened to: "
              << nickname);
  }
  return true;
}

void Command::sendAuthReplies_(Client& client) {
  client.appendToSendBuffer(
      RPL_WELCOME_001(serverHostname_g, client.getNickname(),
                      client.getUserName(), client.getHost()));
  client.appendToSendBuffer(
      RPL_YOURHOST_002(serverHostname_g, IRC_SERVER_VERSION));
  std::string time = std::string(ctime(&serverStartTime_));
  time.pop_back();  // Remove the newline character
  client.appendToSendBuffer(RPL_CREATED_003(serverHostname_g, time));
  client.appendToSendBuffer(RPL_MYINFO_004(serverHostname_g, IRC_SERVER_VERSION,
                                           SUPPORTED_USER_MODES,
                                           SUPPORTED_CHANNEL_MODES));
}

}  // namespace irc
