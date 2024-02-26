/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Command.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: djames <djames@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/21 10:42:51 by djames            #+#    #+#             */
/*   Updated: 2024/02/23 15:56:46 by djames           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Command.h"

extern std::string serverHostname_g;

namespace irc {
// I was thinking here we can put it in a map and acording to the comand we create the response for example ping and pong
std::map<std::string, std::function<void(Command*, Client&)>>
    Command::commands = {{"ping",
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
                         {"kick",
                          [](Command* cmd, Client& client) {
                            cmd->actionPart(client);
                          }},
                         {"nick", [](Command* cmd, Client& client) {
                            cmd->actionNick(client);
                          }}};

Command::Command(const Message& commandString, Client& client,
                 std::map<int, Client>& myClients)
    : client_(client), myClients_(myClients) {
  numeric_ = 0;
  parseCommand(commandString, client);
}

Command::~Command() {}

void Command::execute(Client& client) {
  auto it = commands.find(commandName_);
  if (it != commands.end()) {
    it->second(this, client);
  } else {
    LOG_DEBUG("Command not found: " << commandName_);
  }
}

void Command::parseCommand(const Message& commandString, Client& client) {
  commandName_ = commandString.getCommand();
  prefix_ = commandString.getPrefix();
  param_ = commandString.getParameters();
  numeric_ = commandString.getNumeric();
  execute(client);
}

void Command::actionPing(Client& client) {
  std::string response = ":" + serverHostname_g + " PONG " + serverHostname_g +
                         " :" + client_.getNickname();
  sendRawMessage(client.getFd(), response);
  //:sakura.jp.as.dal.net PONG sakura.jp.as.dal.net :pepit
  LOG_DEBUG(response);
}

void Command::actionChannel(Client& client) {
  std::string response = ":" + serverHostname_g + " #newchannel " +
                         serverHostname_g + " :" + client_.getNickname();
  sendRawMessage(client.getFd(), response);
  LOG_DEBUG(response);
}

void Command::actionPart(Client& client) {
  std::string response = ":" + serverHostname_g + " #newchannel " +
                         serverHostname_g + " :" + client_.getNickname();
  sendRawMessage(client.getFd(), response);
  LOG_DEBUG(response);
}

void Command::actionMode(Client& client) {
  std::string response = ":" + serverHostname_g + " #newchannel " +
                         serverHostname_g + " :" + client_.getNickname();
  sendRawMessage(client.getFd(), response);
  // here we need to put the four parts
  LOG_DEBUG(response);
}

void Command::actionKick(Client& client) {
  std::string response = ":" + serverHostname_g + " #newchannel " +
                         serverHostname_g + " :" + client.getNickname();

  LOG_DEBUG(response);
}

void Command::actionNick(Client& client) {
  if (!(nickCorrectFormat(client.getNickname()))) {
    if (numeric_ == ERR_NONICKNAMEGIVEN) {
      std::string answer =":No nickname given\n";
      client.appendToSendBuffer(answer);
    } else if (numeric_ == ERR_NONICKNAMEGIVEN) {
      std::string erronnick = param_[0] + " :Erroneous nickname\n";
      client.appendToSendBuffer(erronnick);
    }
    return;
  }
  if (findClientByNickname(client.getNickname())) {
    std::string nickExist = param_[0] + " :Nickname is already in use\n";
    client.appendToSendBuffer(nickExist);
    std::cout << client.getSendBuffer();
    return;
  }
  if (checkconnnect()) {
    LOG_DEBUG(
        "you need to set first the pass or the user");  // this i am not sure how to do it yet
  }
  client_.setNickname(param_[0]);
  std::string response = ":" + client_.getOldNickname() + "!" +
                         client.getUserName() + "@" + serverHostname_g +
                         " NICK :" + client_.getNickname() + "\n";
  client.appendToSendBuffer(response);
  LOG_DEBUG(response);
}

bool Command::checkconnnect() {
  int i = 0;
  if (client_.isGotPassword())
    i++;
  if (client_.isGotUser())
    i++;
  if (i >= 1)
    return true;
  return false;
}

bool Command::findClientByNickname(const std::string& nickname) {
  for (std::map<int, Client>::const_iterator it = myClients_.begin();
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

void Command::sendRawMessage(int clientSocket, const std::string& message) {
  const char* msgPtr = message.c_str();
  size_t msgLen = message.length();
  ssize_t bytesSent = 0;

  while (msgLen > 0) {
    bytesSent = send(clientSocket, msgPtr, msgLen, 0);
    if (bytesSent == -1) {
      LOG_DEBUG("Error sending message to client.");
      return;
    }
    msgPtr += bytesSent;
    msgLen -= bytesSent;
  }
}

}  // namespace irc
