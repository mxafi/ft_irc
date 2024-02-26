/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: djames <djames@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/12 16:12:46 by djames            #+#    #+#             */
/*   Updated: 2024/02/23 11:25:46 by djames           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_H
#define CLIENT_H

#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string>
#include <vector>
#include "../common/log.h"
#include "../common/magicNumber.h"

namespace irc {

class Client {
 public:
  Client(int clientSocket);

  ~Client();
  int getFd() const;

  void setSendBuffer(const std::string& sendBuffer);
  std::string getSendBuffer() const;
  void setRecvBuffer(const std::string& recvBuffer);
  std::string getRecvBuffer() const;

  std::string getNickname() const;
  std::string getOldNickname() const;
  void setNickname(const std::string& newNickname);
  void setUserName(const std::string& userName);
  void setPassword(const std::string& password);
  void appendToSendBuffer(std::string& message);
  void appendToRecvdBuffer(const std::string& message);
  void clearSendBuffer();
  void clearRecvdBuffer();
  std::string getUserName() const;
  bool isGotUser() const;
  bool isGotNick() const;
  bool isGotPassword() const;
  bool getAuthenticated();

 private:
  void setOldNickname(const std::string& oldNickname);
  int fd_;
  std::string nickname_;
  std::string oldNickname_;
  std::string userName_;
  std::string sendBuffer_;
  std::string recvBuffer_;
  std::string password_;

  struct ClientStatus {
    bool gotUser;
    bool gotNick;
    bool gotPassword;
    bool authenticated;
  };

  ClientStatus status_;
};

}  // namespace irc

#endif