/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Command.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: djames <djames@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/21 10:42:29 by djames            #+#    #+#             */
/*   Updated: 2024/02/21 17:12:50 by djames           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef COMMAND_H
#define COMMAND_H

#include <functional>
#include <iostream>
#include <map>
#include <string>
#include "../client/Client.h"
#include "../common/log.h"
#include "../common/magicNumber.h"

#define MAX_BUFFER_SIZE 1024  //maybe this we change

namespace irc {

class Command {
 public:
  Command(const std::string& commandString, Client& client);
  void execute();
  void actionPing();
  void actionChannel();
  ~Command();

 private:
  std::string commandName_;
  static std::map<std::string, std::function<void(Command*)>> commands;
  void parseCommand(const std::string& commandString);
  void sendRawMessage(int clientSocket, const std::string& message);
  Client client_;
};
}  // namespace irc

#endif