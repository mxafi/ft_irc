#include "Command.h"

extern std::string serverHostname_g;

namespace irc {
// I was thinking here we can put it in a map and acording to the comand we create the response for example ping and pong
std::map<std::string, std::function<void(Command*, Client&)>>  //this is auto
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

Command::Command(
    const Message& commandString, Client& client,
    std::map<int, Client>& myClients, std::string& password,
    time_t& serverStartTime)  // stsd::map<std::string, &clients myclients>
    : client_(client),
      myClients_(myClients),
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
      LOG_DEBUG("Command not found: " << commandName_);
    }
    return;
  }

  if (commandName_ == "NICK" || commandName_ == "USER" ||
      commandName_ == "PASS") {
    auto it = commands.find(commandName_);
    if (it != commands.end()) {
      it->second(this, client);
    } else {
      LOG_DEBUG("Command not found: " << commandName_);
    }
    return;
  }

  if (commandName_ == "CAP") {
    return;
  }

  // If the client is sending a command before being authenticated
  client.appendToSendBuffer(RPL_ERR_NOTREGISTERED_451(serverHostname_g));
}

void Command::parseCommand(const Message& commandString, Client& client) {
  commandName_ = commandString.getCommand();
  prefix_ = commandString.getPrefix();
  param_ = commandString.getParameters();  //
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
    LOG_DEBUG("Password incorrect");
    LOG_DEBUG("param: " << param_.at(0));
    LOG_DEBUG("pass_: " << pass_);
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
        RPL_ERR_ERR_NICKNAMEINUSE_433(serverHostname_g, param_.at(0)));
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
  std::string response = ":" + serverHostname_g + " #newchannel " +
                         serverHostname_g + " :" + client.getNickname();

  LOG_DEBUG(response);
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
  // TODO: Send a QUIT message to all channels the client is in
  client.appendToSendBuffer(ERR_MESSAGE("Bye, see you soon!"));
  client.setWantDisconnect();
}

void Command::actionJoin(Client& client) {
  (void)client;
}

/****
    * According to PRIVMSG part 3.3.1 on RFC2812, a target can be:
    * - a nick OR #channel
    * - a user@servername
    * - a user%userhost@servername
    * - a user%userhost
    * - a nick!user@userhost
    * -> No delimiter means that it is either a nick or a username
    *    ! is followed by a username
    *    % is followed by a servername
    *    @ is followed by a hostname
    */
void Command::actionPrivmsg(Client& client) {
  size_t amountParameters = param_.size();
  if (amountParameters == 0) {
    client.appendToSendBuffer(RPL_ERR_NORECIPIENT_411(serverHostname_g));
    LOG_DEBUG("CMD::PRIVMSG::NO RECIPIENT and NO MESSAGE");
    return;
  }
  if (amountParameters > 2) {
    client.appendToSendBuffer(RPL_ERR_TOOMANYTARGETS_407(serverHostname_g));
    LOG_DEBUG("CMD::PRIVMSG::TOO MANY TARGETS");
    return;
  }
  if (amountParameters == 1) {
    client.appendToSendBuffer(RPL_ERR_NOTEXTTOSEND_412(serverHostname_g));
    LOG_DEBUG("CMD::PRIVMSG::NO TEXT (1 param only)");
    return;
  }

  if (param_.at(1).front() != ':') {
    client.appendToSendBuffer(RPL_ERR_NOTEXTTOSEND_412(serverHostname_g));
    LOG_DEBUG("CMD::PRIVMSG::NO TEXT (2nd param missing colon)");
    return;
  }

  // else if (
  //     // Message contains  only the trailing parameter colon ':' delimiter
  //     param_.at(1).front() == ':' && param_.at(1).length() == 1) {
  //   RPL_ERR_NOTEXTTOSEND_412(serverHostname_g);
  //   return;
  // }
  // if (param_.at(0).find(
  //         CHANNEL_PREFIXES)) {  // checks if target is containing a prefix character
  //   if (validateChannel() == FALSE) {  // TODO: implementation
  //     RPL_ERR_NOSUCHCHANNEL_403(serverHostname_g);
  //     return;
  //   } else if (senderAllowedToMsg() == FALSE) {  // TODO: implementation
  //     RPL_ERR_CANNOTSENDTOCHAN_404(serverHostname_g, param_.at(0));
  //     return;
  //   }
  // } else {


  std::string messageWithoutColon = param_.at(1).erase(0, 1);
  int target = findClientByNickname(param_.at(0));
  std::string targetRecipient = param_.at(0);
  if (target == 0) {
    client.appendToSendBuffer(RPL_ERR_NOSUCHNICK_401(serverHostname_g, targetRecipient));
    LOG_DEBUG("CMD::PRIVMSG::findClientByNickname: USER NOT FOUND");
    return;
  }
  LOG_DEBUG("CMD::PRIVMSG: Message is :" + messageWithoutColon + " from " +
            client.getNickname() + " to " + targetRecipient);

  std::string formattedSender =
      FORMAT_NICK_USER_HOST(client.getNickname(), client.getUserName(),
                            "senderHost");  // TODO: fix in channel branch
  std::string privmsg =
      PRIVMSG_FORMAT(formattedSender, targetRecipient, messageWithoutColon);
  myClients_.find(target)->second.appendToSendBuffer(privmsg);
}

int Command::findClientByNickname(const std::string& nickname) {
  std::string lowerNickname =
      nickname;  //here we just put everything in lowercase
  std::transform(lowerNickname.begin(), lowerNickname.end(),
                 lowerNickname.begin(), ::tolower);
  std::replace(lowerNickname.begin(), lowerNickname.end(), '{', '[');
  std::replace(lowerNickname.begin(), lowerNickname.end(), '}', ']');
  std::replace(lowerNickname.begin(), lowerNickname.end(), '|', '\\');
  std::replace(lowerNickname.begin(), lowerNickname.end(), '^', '~');

  for (std::map<int, Client>::const_iterator it = myClients_.begin();
       it != myClients_.end(); ++it) {
    std::string clientNickname =
        it->second.getNickname();  //here we just put everything in lowercase
    std::transform(clientNickname.begin(), clientNickname.end(),
                   clientNickname.begin(), ::tolower);
    std::replace(clientNickname.begin(), clientNickname.end(), '{', '[');
    std::replace(clientNickname.begin(), clientNickname.end(), '}', ']');
    std::replace(clientNickname.begin(), clientNickname.end(), '|', '\\');
    std::replace(clientNickname.begin(), clientNickname.end(), '^', '~');
    if (clientNickname == lowerNickname) {
      LOG_DEBUG("Found client with the same nickname: " << nickname);
      return it->second
          .getFd();  // Return the file descriptor of the found client
    }
  }

  return 0;
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
                      client.getUserName(), client.getIpAddr()));
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
