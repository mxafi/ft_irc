#include "Client.h"

namespace irc {

Client::Client(int fd, const struct sockaddr& sockaddr)
    : fd_(fd), sockaddr_(sockaddr) {
  status_.gotUser = false;
  status_.gotNick = false;
  status_.gotPassword = false;
  status_.authenticated = false;
  status_.wantDisconnect = false;
}

Client::~Client() {}

bool Client::isAuthenticated() {
  return status_.authenticated;
}

void Client::setNickname(const std::string& newNickname) {
  if (!status_.gotNick) { // if the client is setting their nickname for the first time when they connect
    nickname_ = (newNickname.size() > NICK_MAX_LENGTH_RFC2812)
                    ? newNickname.substr(0, NICK_MAX_LENGTH_RFC2812)
                    : newNickname;
    setOldNickname_(nickname_);
    status_.gotNick = true;
    if (status_.gotUser && status_.gotPassword) {
      status_.authenticated = true;
      LOG_DEBUG("client is authenticated")
    }
  } else { // if the client is changing their nickname
    setOldNickname_(nickname_);
    nickname_ = (newNickname.size() > NICK_MAX_LENGTH_RFC2812)
                    ? newNickname.substr(0, NICK_MAX_LENGTH_RFC2812)
                    : newNickname;
  }
  LOG_DEBUG("nickname is set to: " << nickname_);
}

void Client::setOldNickname_(const std::string& oldNickname) {
  oldNickname_ = oldNickname;
}

void Client::setUserName(const std::string& userName) {
  userName_ = userName;
  status_.gotUser = true;
  if (status_.gotPassword && status_.gotNick) {
    status_.authenticated = true;
    LOG_DEBUG("client is authenticated")
  }
  LOG_DEBUG("user is set to: " << userName_);
}

void Client::setPassword(const std::string& password) {
  password_ = password;
  status_.gotPassword = true;
  if (status_.gotUser && status_.gotNick) {
    status_.authenticated = true;
    LOG_DEBUG("client is authenticated")
  }
  LOG_DEBUG("password is set to: " << password_);
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

int Client::getFd() const {
  return fd_;
}

std::string Client::getIpAddr() const {
  char ip[INET6_ADDRSTRLEN];
  struct sockaddr_in* sa = (struct sockaddr_in*)&sockaddr_;
  inet_ntop(AF_INET, &sa->sin_addr, ip, INET6_ADDRSTRLEN);
  LOG_DEBUG("client got ip address: " << ip);
  return std::string(ip);
}

void Client::setSendBuffer(const std::string& sendBuffer) {
  this->sendBuffer_ = sendBuffer;
}

std::string& Client::getSendBuffer() {
  return sendBuffer_;
}

void Client::setRecvBuffer(const std::string& recvBuffer) {
  this->recvBuffer_ = recvBuffer;
}

std::string& Client::getRecvBuffer() {
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

void Client::appendToSendBuffer(const std::string& packet) {
  sendBuffer_ += packet;
}

void Client::appendToRecvBuffer(const std::string& packet) {
  sendBuffer_ += packet;
}

void Client::clearSendBuffer() {
  sendBuffer_.clear();
}

void Client::clearRecvdBuffer() {
  sendBuffer_.clear();
}

void Client::setWantDisconnect() {
  status_.wantDisconnect = true;
}

bool Client::getWantDisconnect() const {
  return status_.wantDisconnect;
}

}  // namespace irc
