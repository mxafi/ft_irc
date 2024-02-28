#include "Command.h"

extern std::string serverHostname_g;

namespace irc {
// I was thinking here we can put it in a map and acording to the comand we create the response for example ping and pong
std::map<std::string, std::function<void(Command*, Client&)>>  //this is auto
    Command::commands = {{"PING",
                          [](Command* cmd, Client& client) {
                            cmd->actionPing(client);
                          }},
                         {"channel",
                          [](Command* cmd, Client& client) {
                            cmd->actionChannel(client);
                          }},
                         {"part",
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
                         {"privmsg",
                          [](Command* cmd, Client& client) {
                            cmd->actionPrivmsg(client);
                          }},
                         {"JOIN", [](Command* cmd, Client& client) {
                            cmd->actionJoin(client);
                          }}};

Command::Command(
    const Message& commandString, Client& client,
    std::map<int, Client>& myClients,
    std::string& password)  // stsd::map<std::string, &clients myclients>
    : client_(client), myClients_(myClients), pass_(password) {
  numeric_ = 0;
  parseCommand(commandString, client);
}

Command::~Command() {}

// void Command::execute(Client& client) {
//   auto it = commands.find(commandName_);  // I will change
//   if (it != commands.end()) {
//     it->second(this, client);
//   } else {
//     LOG_DEBUG("Command not found: " << commandName_);  // answer with numeric
//   }
// }
void Command::execute(Client& client) {
  if (client.isAuthenticated()) {
    auto it = commands.find(commandName_);  // I will change
    if (it != commands.end()) {
      it->second(this, client);
    } else {
      LOG_DEBUG("Command not found: " << commandName_);  // answer with numeric
    }
  } else if (!(commandName_.compare("NICK") || commandName_.compare("USER") ||
               commandName_.compare("PASS"))) {
    auto it = commands.find(commandName_);  // I will change
    if (it != commands.end()) {
      it->second(this, client);
    } else {
      LOG_DEBUG("Command not found: " << commandName_);
    }
  } else {
    LOG_DEBUG("the client ";
  }
}

void Command::parseCommand(const Message& commandString, Client& client) {
  commandName_ = commandString.getCommand();
  prefix_ = commandString.getPrefix();
  param_ = commandString.getParameters();  //
  numeric_ = commandString.getNumeric();
  execute(client);
}

void Command::actionPing(Client& client) {
  std::string response = ":" + serverHostname_g + " PONG " + serverHostname_g +
                         " :" + client.getNickname();
  client.appendToSendBuffer(response);
  //:sakura.jp.as.dal.net PONG sakura.jp.as.dal.net :pepit
  LOG_DEBUG(response);
}

void Command::actionPass(Client& client) {
  if (client.isAuthenticated()) {
    return;
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

void Command::actionNick(
    Client& client) {  // we are not printing any error in the server
  // if (checkconnnect()) {
  //   if (!(client.getAuthenticated())) {
  //     if (!(client.isGotPassword())) {
  //       LOG_DEBUG(
  //           "you need to set first the password");  // what should we append to the client we decide to check wwith the password first.
  //       return;
  //     }
  //   }
  // }
  client_.setNickname(param_[0]);
  // if (!(nickCorrectFormat(client.getNickname()))) {//
  //   if (numeric_ == ERR_NONICKNAMEGIVEN) {
  //     std::string answer = ":No nickname given\r\n";
  //     client.appendToSendBuffer(answer);
  //   } else if (numeric_ == ERR_ERRONEUSNICKNAME) {
  //     std::string erronnick = param_[0] + " :Erroneous nickname\r\n";//fix
  //     client.appendToSendBuffer(erronnick);
  //   }
  //   return;
  // }
  // if (findClientByNickname(client.getNickname())) {
  //   std::string nickExist = param_[0] + " :Nickname is already in use\r\n";// fix
  //   client.appendToSendBuffer(nickExist);
  //   return;
  // }
  client_.setNickname(param_[0]);
  std::string response = ":" + client_.getOldNickname() + "!" +
                         client.getUserName() + "@" + serverHostname_g +
                         " NICK :" + client_.getNickname() + "\r\n";
  client.appendToSendBuffer(response);
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

  std::string replyJoin;
  replyJoin = ":" + serverHostname_g +
              " 451 * JOIN :You must finish connecting with another nickname "
              "first.\r\n";
  client.appendToSendBuffer(replyJoin);
  LOG_DEBUG("Connection was made");
  // replyJoin = ":" + serverHostname_g + " 001 " + client.getUserName() + ":Welcome to the DIEGOnet IRC Network " + client.getNickname()+ "!~" + client.getUserName() + client.getIpAddr()+ "\r\n";
  // client.appendToSendBuffer(replyJoin);
  //:lair.nl.eu.dal.net 001 diegoo :Welcome to the DALnet IRC Network diegoo!~djames@194.136.126.51
  // replyJoin = ":" + serverHostname_g + " 002 " + client.getUserName() + ":This server was created Now\r\n"; // check the date function
  // client.appendToSendBuffer(replyJoin);
  // replyJoin = ":" + serverHostname_g + " 003 " + client.getUserName() + ":This server was created Now\r\n";
  // client.appendToSendBuffer(replyJoin);
  // replyJoin = ":" + serverHostname_g + " 004 " + client.getUserName() + serverHostname_g + "Diego2.2\r\n";
  // client.appendToSendBuffer(replyJoin);
  //   :lair.nl.eu.dal.net NOTICE AUTH :*** Looking up your hostname...
  // >> :lair.nl.eu.dal.net NOTICE AUTH :*** Checking Ident
  // >> :lair.nl.eu.dal.net NOTICE AUTH :*** Found your hostname
  // >> :lair.nl.eu.dal.net NOTICE AUTH :*** No Ident response
  // >> :lair.nl.eu.dal.net 451 * JOIN :You must finish connecting with another nickname first.

  // >> :lair.nl.eu.dal.net 002 diegoo :Your host is lair.nl.eu.dal.net, running version bahamut-2.2.3
  // >> :lair.nl.eu.dal.net 003 diegoo :This server was created Tue Dec 19 2023 at 22:45:40 CET
  // >> :lair.nl.eu.dal.net 004 diegoo lair.nl.eu.dal.net bahamut-2.2.3 aAbcCdefFghHiIjkKmnoOPrRsSwxXy AbceiIjklLmMnoOpPrRsStv beIjklov
  // 001    RPL_WELCOME
  //               "Welcome to the Internet Relay Network
  //                <nick>!<user>@<host>"
  //        002    RPL_YOURHOST
  //               "Your host is <servername>, running version <ver>"
  //        003    RPL_CREATED
  //               "This server was created <date>"
  //        004    RPL_MYINFO
  //               "<servername> <version> <available user modes>
  //                <available channel modes>"

  // we dont send anything to the client wew jusgt set it up
  //:syrk!kalt@millennium.stealth.net QUIT :Gone to have lunch ; User
  //                                syrk has quit IRC to have lunch.
  // LOG_DEBUG("user name is set");
}

void Command::actionPrivmsg(Client& client) {

  std::string replyPrivmsg = "here you put \r\n";

  client.appendToSendBuffer(replyPrivmsg);

}

void Command::actionUser(Client& client) {}

bool Command::findClientByNickname(const std::string& nickname) {
  for (std::map<int, Client>::const_iterator it =
           myClients_.begin();  // it could be auto
       it != myClients_.end(); ++it) {
    if (it->second.getNickname() == nickname) {
      LOG_DEBUG("Found client with nickname: " << nickname);
      numeric_ = ERR_NICKNAMEINUSE;
      return true;
    }
  }
  return false;
}

bool Command::nickCorrectFormat(const std::string& str) {
  if (str.empty()) {
    LOG_DEBUG("the nickname is empty");
    numeric_ = ERR_NONICKNAMEGIVEN;
    return false;
  }
  char firstChar = str.front();
  if (std::isdigit(static_cast<unsigned char>(firstChar))) {
    LOG_DEBUG("the first character of the nickname is a digit");
    numeric_ = ERR_ERRONEUSNICKNAME;
    return false;
  }
  if ((str.find('#') != std::string::npos)) {
    LOG_DEBUG("the nickname has #");
    numeric_ = ERR_ERRONEUSNICKNAME;
    return false;
  }
  if (str.find(':') != std::string::npos) {
    LOG_DEBUG("the nickname has :");
    numeric_ = ERR_ERRONEUSNICKNAME;
    return false;
  }
  if (str.find(' ') != std::string::npos) {
    LOG_DEBUG("the nickname has a space");
    numeric_ = ERR_ERRONEUSNICKNAME;
    return false;
  }

  return true;
}

// void Command::sendRawMessage(int clientSocket, const std::string& message) {
//   const char* msgPtr = message.c_str();
//   size_t msgLen = message.length();
//   ssize_t bytesSent = 0;

//   while (msgLen > 0) {
//     bytesSent = send(clientSocket, msgPtr, msgLen, 0);
//     if (bytesSent == -1) {
//       LOG_DEBUG("Error sending message to client.");
//       return;
//     }
//     msgPtr += bytesSent;
//     msgLen -= bytesSent;
//   }
// }

}  // namespace irc
