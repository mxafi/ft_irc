/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Command.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: djames <djames@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/21 10:42:29 by djames            #+#    #+#             */
/*   Updated: 2024/02/27 11:54:42 by djames           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef COMMAND_H
#define COMMAND_H

#include <cctype>
#include <functional>
#include <iostream>
#include <map>
#include <string>
#include "../client/Client.h"
#include "../common/log.h"
#include "../common/magicNumber.h"
#include "../message/Message.h"

#define MAX_BUFFER_SIZE 1024  //maybe this we change

namespace irc {

class Command {
 public:
  Command(const Message& commandString, Client& client,
          std::map<int, Client>& myClients);
  void execute(Client& client);
  void actionPing(Client& client);
  void actionChannel(Client& client);
  void actionKick(Client& client);
  void actionMode(Client& client);
  void actionPart(Client& client);
  void actionNick(Client& client);
  void actionUser(Client& client);
  void actionQuit(Client& client);
  void actionPrivmsg(Client& client);
  void actionJoin(Client& client);
  ~Command();

 private:
  std::string commandName_;
  bool findClientByNickname(const std::string& nickname);
  bool nickCorrectFormat(const std::string& str);
  static std::map<std::string, std::function<void(Command*, Client&)>> commands;
  void parseCommand(const Message& commandString, Client& client);
  //void sendRawMessage(int clientSocket, const std::string& message);
  bool checkconnnect();
  Client client_;
  std::string prefix_;
  std::vector<std::string> param_;
  int numeric_;
  std::map<int, Client>& myClients_;
};
}  // namespace irc

#endif