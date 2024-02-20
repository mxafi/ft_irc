/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: djames <djames@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/20 10:44:08 by djames            #+#    #+#             */
/*   Updated: 2024/02/20 11:15:05 by djames           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.h"

Client::Client(int clientSocket) : socket_(clientSocket) {
  flag_ = 0;
}

Client::~Client() {
  if (socket_ != -1) {
    close(socket_);
  }
}

void Client::setNickname(const std::string& newNickname) {
    setOldNickname(nickname_);
    nickname_ = (newNickname.size() > 9) ? newNickname.substr(0, 9) : newNickname;
    if(flag_ == 0)
    {
      flag_ =1;
      setOldNickname(nickname_);
    }
}
void Client::setOldNickname(const std::string& oldNickname) {
    oldNickname_ = oldNickname;
}
void Client::setUserName(const std::string& userName) {
    userName_ = userName;
}

std::string Client::getNickname() const {
    return nickname_;
}
std::string Client::getOldNickname() const {
    return oldNickname_;
}
std::string Client::getUserName() const {
    return userName_;
}

int Client::getSocket() const {
  return socket_;
}
