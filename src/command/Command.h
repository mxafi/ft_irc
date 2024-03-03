#ifndef COMMAND_H
#define COMMAND_H

#include <cctype>
#include <functional>
#include <iostream>
#include <map>
#include <regex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "../channel/Channel.h"
#include "../client/Client.h"
#include "../common/log.h"
#include "../common/magicNumber.h"
#include "../common/reply.h"
#include "../message/Message.h"

#define MAX_BUFFER_SIZE 1024  //maybe this we change

namespace irc {

class Command {
 public:
  Command(const Message& commandString, Client& client,
          std::map<int, Client>& myClients, std::string& password,
          time_t& serverStartTime);
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
  void actionPass(Client& client);
  ~Command();

 private:
  std::string commandName_;
  bool findClientByNickname(const std::string& nickname);
  void parseCommand(const Message& commandString, Client& client);
  void sendAuthReplies_(Client& client);
  static std::map<std::string, std::function<void(Command*, Client&)>> commands;
  bool isValidNickname(std::string& nickname);
  Client client_;
  std::string prefix_;
  std::vector<std::string> param_;
  int numeric_;
  std::map<int, Client>& myClients_;
  std::string& pass_;
  time_t serverStartTime_;
};
}  // namespace irc

#endif