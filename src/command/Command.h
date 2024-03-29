#ifndef COMMAND_H
#define COMMAND_H

#include <cctype>
#include <functional>
#include <iostream>
#include <map>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#include "../channel/Channel.h"
#include "../client/Client.h"
#include "../common/log.h"
#include "../common/magicNumber.h"
#include "../common/reply.h"
#include "../message/Message.h"

#define MAX_BUFFER_SIZE 1024  //maybe this we change

extern std::string serverHostname_g;

namespace irc {

class Command {
   public:
    Command(const Message& commandString, Client& client, std::map<int, Client>& allClients, std::string& password, time_t& serverStartTime,
            std::map<std::string, Channel>& allChannels);
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
    void actionTopic(Client& client);
    void actionInvite(Client& client);
    ~Command();

   private:
    std::string prefix_;
    std::string commandName_;
    std::vector<std::string> param_;
    int numeric_;
    Client& client_;
    std::map<int, Client>& allClients_;
    std::map<std::string, Channel>& allChannels_;
    std::string& pass_;
    time_t serverStartTime_;

    Client& findClientByNicknameOrThrow(const std::string& nickname);
    void sendAuthReplies_(Client& client);
    static std::map<std::string, std::function<void(Command*, Client&)>> commands;
    bool isValidNickname(std::string& nickname);

    // PRIVMSG
    bool validateTarget();
};
}  // namespace irc

#endif
