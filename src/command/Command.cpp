/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Command.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: djames <djames@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/21 10:42:51 by djames            #+#    #+#             */
/*   Updated: 2024/02/22 14:30:05 by djames           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Command.h"

extern std::string serverHostname_g;

namespace irc {
// I was thinking here we can put it in a map and acording to the comand we create the response for example ping and pong
std::map<std::string, std::function<void(Command*)>> Command::commands = {
    {"ping",
     [](Command* cmd) {
       cmd->actionPing();
     }},
    {"channel",
     [](Command* cmd) {
       cmd->actionChannel();
     }},
    {"part", [](Command* cmd) {
       cmd->actionPart();
     }},{"kick", [](Command* cmd) {
       cmd->actionPart();
     }}, {"part", [](Command* cmd) {
       cmd->actionPart();
     }},{"nick", [](Command* cmd) {
       cmd->actionNick();
     }}  };

Command::Command(const Message& commandString, Client& client)
    : client_(client) {
  parseCommand(commandString);
}

Command::~Command() {}

void Command::execute() {
  std::map<std::string, std::function<void(Command*)>>::iterator it =
      commands.find(commandName_);
  if (it != commands.end()) {

    it->second(this);
  } else {
    LOG_DEBUG("not found command : " << commandName_);
  }
}

void Command::parseCommand(
    const Message&
        commandString) {  // tthis parssing is whatever but maybe we can talk with the messague waht twill come here message what to do
  commandName_ = commandString.getCommand();
  prefix_ = commandString.getPrefix();
  param_ = commandString.getParameters();
  numeric_ = commandString.getNumeric();
  
}

void Command::actionPing() {
  std::string response = ":" + serverHostname_g + " PONG " + serverHostname_g +
                         " :" + client_.getNickname();
  sendRawMessage(client_.getFd(), response);
  //:sakura.jp.as.dal.net PONG sakura.jp.as.dal.net :pepit
  LOG_DEBUG(response);
}

void Command::actionChannel() {
  std::string response = ":" + serverHostname_g + " #newchannel " +
                         serverHostname_g + " :" + client_.getNickname();
  sendRawMessage(client_.getFd(), response);
  LOG_DEBUG(response);
}

void Command::actionPart() {
  std::string response = ":" + serverHostname_g + " #newchannel " +
                         serverHostname_g + " :" + client_.getNickname();
  sendRawMessage(client_.getFd(), response);
  LOG_DEBUG(response);
}

void Command::actionMode() {
  std::string response = ":" + serverHostname_g + " #newchannel " +
                         serverHostname_g + " :" + client_.getNickname();
  sendRawMessage(client_.getFd(), response);
  // here we need to put the four parts
  LOG_DEBUG(response);
}

void Command::actionKick() {
  std::string response = ":"  + serverHostname_g + " #newchannel " +
                         serverHostname_g + " :" + client_.getNickname();
  
  LOG_DEBUG(response);
}

void Command::actionNick() {
  //check validity
  client_.setNickname(param_[0]);
  std::string response = ":" + client_.getOldNickname() + "!" + client_.getUserName() + "@" + serverHostname_g + " NICK " +
                         client_.getNickname();
  sendRawMessage(client_.getFd(), response);
  //:pepito645!~djames@dbc-958a-261c-eb6a-813d.136.194.ip NICK :pepit
  LOG_DEBUG(response);
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
