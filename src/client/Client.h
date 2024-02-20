/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: djames <djames@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/12 16:12:46 by djames            #+#    #+#             */
/*   Updated: 2024/02/20 16:10:47 by djames           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_H
#define CLIENT_H

#include <sys/poll.h>
#include <sys/socket.h>
#include <string>
#include <vector>
#include "../common/magicNumber.h"

class Client {
 public:
  Client(int clientSocket);

  ~Client();
  int getSocket() const;

  void setSendBuffer(const std::string& sendBuffer);
  std::string getSendBuffer() const;
  void setRecvBuffer(const std::string& recvBuffer);
  std::string getRecvBuffer() const;

  std::string getNickname() const;
  std::string getOldNickname() const;
  void setOldNickname(const std::string& oldNickname);
  void setNickname(const std::string& newNickname);
  void setUserName(const std::string& userName);
  void appendToSendBuffer(const std::string& message);
  void appendToRecvdBuffer(const std::string& message);
  void clearSendBuffer();
  void clearRecvdBuffer();
  std::string getUserName() const;
  bool isGotUser() const;
  bool isGotNick() const;
  bool isGotPassword() const;
  bool getAuthenticated();

 private:
  int socket_;
  std::string nickname_;
  int clientFd_;
  std::string oldNickname_;
  std::string userName_;
  std::string sendBuffer_;
  std::string recvBuffer_;

  struct ClientStatus {
    bool gotUser;
    bool gotNick;
    bool gotPassword;
    bool authenticated;
  };

  ClientStatus status_;
};

#endif