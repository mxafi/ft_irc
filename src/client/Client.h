/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: djames <djames@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/12 16:12:46 by djames            #+#    #+#             */
/*   Updated: 2024/02/20 11:15:58 by djames           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_H
#define CLIENT_H

#include <sys/poll.h>
#include <string>
#include <vector>
#include <sys/socket.h>

class Client {
 public:
  Client(int clientSocket);

  ~Client();
  int getSocket() const;
  std::string getNickname() const;
  std::string getOldNickname() const;
  void setOldNickname(const std::string& oldNickname);
  void setNickname(const std::string& newNickname);
  void setUserName(const std::string& userName);
  std::string getUserName() const;
  
 private:
  int socket_;
  std::string nickname_;
  int clientFd_;
  std::string oldNickname_;
  std::string userName_;
  int flag_;
};

#endif