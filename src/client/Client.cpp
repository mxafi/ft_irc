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
  if (!status_
           .gotNick) {  // if the client is setting their nickname for the first time when they connect
    nickname_ = (newNickname.size() > NICK_MAX_LENGTH_RFC2812)
                    ? newNickname.substr(0, NICK_MAX_LENGTH_RFC2812)
                    : newNickname;
    setOldNickname_(nickname_);
    status_.gotNick = true;
    if (status_.gotUser && status_.gotPassword) {
      status_.authenticated = true;
      LOG_DEBUG("Client::setNickname: client is authenticated")
    }
  } else {  // if the client is changing their nickname
    setOldNickname_(nickname_);
    nickname_ = (newNickname.size() > NICK_MAX_LENGTH_RFC2812)
                    ? newNickname.substr(0, NICK_MAX_LENGTH_RFC2812)
                    : newNickname;
  }
  LOG_DEBUG("Client::setNickname: nickname is set to: " << nickname_);
}

void Client::setOldNickname_(const std::string& oldNickname) {
  oldNickname_ = oldNickname;
}

void Client::setUserName(const std::string& userName) {
  userName_ = userName;
  status_.gotUser = true;
  if (status_.gotPassword && status_.gotNick) {
    status_.authenticated = true;
    LOG_DEBUG("Client::setUserName: client is authenticated")
  }
  LOG_DEBUG("Client::setUserName: user is set to: " << userName_);
}

void Client::setPassword(const std::string& password) {
  password_ = password;
  status_.gotPassword = true;
  if (status_.gotUser && status_.gotNick) {
    status_.authenticated = true;
    LOG_DEBUG("Client::setPassword: client is authenticated")
  }
  LOG_DEBUG("Client::setPassword: password is set to: " << password_);
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

void Client::populateIpAddr_() {
  char ip[INET6_ADDRSTRLEN];
  struct sockaddr_in* sa = (struct sockaddr_in*)&sockaddr_;
  inet_ntop(AF_INET, &sa->sin_addr, ip, INET6_ADDRSTRLEN);
  LOG_DEBUG("Client::populateIpAddr_: client got ip address: " << ip);
  ipAddr_ = std::string(ip);
  return;
}

std::string& Client::getHost() {
  if (ipAddr_.empty()) {
    populateIpAddr_();
  }
  return ipAddr_;
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
  LOG_DEBUG(
      "Client::appendToSendBuffer: appending message to sendBuffer for nick: "
      << nickname_ << ": " << packet)
  sendBuffer_ += packet;
}

void Client::appendToRecvBuffer(const std::string& packet) {
  LOG_DEBUG(
      "Client::appendToRecvBuffer: appending message to recvBuffer for nick: "
      << nickname_ << ": " << packet)
  recvBuffer_ += packet;
}

void Client::clearSendBuffer() {
  LOG_DEBUG("Client::clearSendBuffer: clearing sendBuffer for nick "
            << nickname_);
  sendBuffer_.clear();
}

void Client::clearRecvBuffer() {
  LOG_DEBUG("Client::clearRecvBuffer: clearing recvBuffer for nick "
            << nickname_);
  recvBuffer_.clear();
}

void Client::setWantDisconnect() {
  LOG_DEBUG("Client::setWantDisconnect: nick " << nickname_
                                               << " wants to disconnect");
  status_.wantDisconnect = true;
}

bool Client::getWantDisconnect() const {
  return status_.wantDisconnect;
}

std::string Client::getDisconnectReason() const {
  return disconnectReason_;
}

void Client::setDisconnectReason(const std::string& reason) {
  LOG_DEBUG("Client::setDisconnectReason: nick"
            << nickname_ << " set disconnectReason: " << reason);
  disconnectReason_ = reason;
}

void Client::recordMyChannel(std::string& channelName) {
  myChannelsByName_.push_back(channelName);
  LOG_DEBUG("Client::recordMyChannel: nick "
            << nickname_ << " recorded channel " << channelName);
}

void Client::unrecordMyChannel(std::string& channelName) {
  std::vector<std::string>::iterator it = myChannelsByName_.begin();
  while (it != myChannelsByName_.end()) {
    if (*it == channelName) {
      myChannelsByName_.erase(it);
      LOG_DEBUG("Client::unrecordMyChannel: nick "
                << nickname_ << " unrecorded channel " << channelName);
      break;
    }
    it++;
  }
}

std::vector<std::string>& Client::getMyChannels() {
  return myChannelsByName_;
}

}  // namespace irc
