/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: djames <djames@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/20 10:44:08 by djames            #+#    #+#             */
/*   Updated: 2024/02/20 16:10:28 by djames           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.h"

Client::Client(int clientSocket) : socket_(clientSocket) {
  status_.gotUser = false;
  status_.gotNick = false;
  status_.gotPassword = false;
  status_.authenticated = false;
}

Client::~Client() {
  if (socket_ != SOCKET_FAILURE) {
    close(socket_);
  }
}

bool Client::getAuthenticated() {
  if (status_.gotNick && status_.gotUser) {
    return true;
  }
  return false;
}

void Client::setNickname(const std::string& newNickname) {
  if (!status_.gotNick) {
    setOldNickname(nickname_);
    nickname_ = (newNickname.size() > NICK_MAX_LENGTH_RFC2812)
                    ? newNickname.substr(0, NICK_MAX_LENGTH_RFC2812)
                    : newNickname;
    status_.gotNick = true;
  }
}

void Client::setOldNickname(const std::string& oldNickname) {
  oldNickname_ = oldNickname;
}

void Client::setUserName(const std::string& userName) {
  userName_ = userName;
  status_.gotUser = true;
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

void Client::setSendBuffer(const std::string& sendBuffer) {
  this->sendBuffer_ = sendBuffer;
}

std::string Client::getSendBuffer() const {
  return sendBuffer_;
}

void Client::setRecvBuffer(const std::string& recvBuffer) {
  this->recvBuffer_ = recvBuffer;
}

std::string Client::getRecvBuffer() const {
  return recvBuffer_;
}

bool Client::isGotUser() const {
  return status_.gotUser;
}

bool Client::isGotNick() const {
  return status_.gotNick;
}

bool Client::isGotPassword() const {
  return status_.gotPassword;
}

void Client::appendToSendBuffer(const std::string& message) {
  sendBuffer_ += message;
}

void Client::appendToRecvdBuffer(const std::string& message) {
  sendBuffer_ += message;
}

void Client::clearSendBuffer() {
  sendBuffer_.clear();
}
void Client::clearRecvdBuffer() {
  sendBuffer_.clear();
}