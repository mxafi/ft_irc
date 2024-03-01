#ifndef CHANNEL_H
#define CHANNEL_H

#include <stdexcept>
#include <string>
#include <vector>

#include "../client/Client.h"

namespace irc {

class Channel {
 public:
  ~Channel();
  Channel(Client& creatorClient, const std::string& name);

  std::string getName() const;
  void sendMessageToMembers(const std::string& message);
  void setTopic(const std::string& topic);
  std::string getTopic() const;
  void setKey(const std::string& key);
  std::string getKey() const;
  void setInviteOnly(bool isInviteOnly);
  bool isInviteOnly() const;
  void setTopicProtected(bool isTopicProtected);
  bool isTopicProtected() const;
  void setUserLimit(int userLimit);
  int getUserLimit() const;
  void joinMember(Client& client);
  void partMember(Client& client);
  bool isMember(Client& client);
  void setOperatorStatus(Client& client, bool setOperatorStatusTo);
  bool isOperator(Client& client);
  void invite(Client& client);
  void uninvite(Client& client);
  bool isInvited(Client& client);

 private:
  std::string name_;
  std::vector<Client*> invited_;
  std::vector<Client*> members_;
  std::vector<Client*> operators_;
  std::string topic_;
  std::string key_;
  bool isInviteOnly_;
  bool isTopicProtected_;
  int userLimit_;
};

}  // namespace irc

#endif
