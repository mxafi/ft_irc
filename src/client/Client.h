#ifndef CLIENT_H
#define CLIENT_H

#include <arpa/inet.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string>
#include <vector>
#include "../common/log.h"
#include "../common/magicNumber.h"

namespace irc {

// Light interdependancy between client and channel
class Channel;

class Client {
 public:
  Client(int fd, const struct sockaddr& sockaddr);

  ~Client();
  int getFd() const;

  void setSendBuffer(const std::string& sendBuffer);
  std::string& getSendBuffer();
  void setRecvBuffer(const std::string& recvBuffer);
  std::string& getRecvBuffer();

  std::string getNickname() const;
  std::string getOldNickname() const;
  void setNickname(const std::string& newNickname);
  void setUserName(const std::string& userName);
  void setPassword(const std::string& password);
  void appendToSendBuffer(const std::string& packet);
  void appendToRecvBuffer(const std::string& packet);
  void clearSendBuffer();
  void clearRecvdBuffer();
  std::string getUserName() const;
  bool isGotUser() const;
  bool isGotNick() const;
  bool isGotPassword() const;
  bool isAuthenticated();
  void setWantDisconnect();
  bool getWantDisconnect() const;
  std::string getIpAddr() const;
  std::string getDisconnectReason() const;
  void setDisconnectReason(const std::string& reason);
  void recordChannel(Channel& channel);
  void removeChannel(Channel& channel);
  std::vector<Channel*>& getChannels();

 private:
  void setOldNickname_(const std::string& oldNickname);
  int fd_;
  struct sockaddr sockaddr_;

  std::string nickname_;
  std::string oldNickname_;
  std::string userName_;
  std::string sendBuffer_;
  std::string recvBuffer_;
  std::string password_;
  std::string disconnectReason_;

  std::vector<Channel*> channels_;

  struct ClientStatus {
    bool gotUser;
    bool gotNick;
    bool gotPassword;
    bool authenticated;
    bool wantDisconnect;
  };

  ClientStatus status_;
};

}  // namespace irc

#endif