/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Command.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: djames <djames@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/21 10:42:51 by djames            #+#    #+#             */
/*   Updated: 2024/02/21 15:30:02 by djames           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Command.h"

extern std::string serverHostname_g;

namespace irc {
// I was thinking here we can put it in a map and acording to the comand we create the response for example ping and pong
std::map<std::string, std::function<void(Command*)>> Command::commands = {
    {"PING",
     [](Command* cmd) {
       cmd->actionPing();
     }},
    {"Channel", [](Command* cmd) {
       cmd->actionChannel();
     }}};

Command::Command(const std::string& commandString, Client& client)
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
    const std::string&
        commandString) {  // tthis parssing is whatever but maybe we can talk with the messague waht twill come here
  std::size_t pos = commandString.find(' ');
  if (pos != std::string::npos) {
    commandName_ = commandString.substr(0, pos);
  } else {
    commandName_ = commandString;
  }
}

void Command::actionPing() {
  std::string response = ":" + serverHostname_g + " PONG " + serverHostname_g +
                         " :" + client_.getNickname();
  LOG_DEBUG(response);
}

void Command::actionChannel() {
  std::string response = ":" + serverHostname_g + " #newchannel " +
                         serverHostname_g + " :" + client_.getNickname();
  LOG_DEBUG(response);
}

}  // namespace irc
