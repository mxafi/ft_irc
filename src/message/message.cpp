#include "message.h"

namespace irc {
Message::Message(const std::string& sender, const std::string& receiver,
                 const std::string& command)
    : sender_(sender), receiver_(receiver), command_(command){};

Message::Message(const std::string& sender, const std::string& receiver,
                 const std::string& command,
                 const std::string& parameters)
    : sender_(sender),
      receiver_(receiver),
      command_(command),
      parameters_(parameters) {}

Message::~Message(){};

std::string Message::getSender() const {
  return sender_;
}

std::string Message::getReceiver() const {
  return receiver_;
}

std::string Message::getCommand() const {
  return command_;
}

std::string Message::getParameters() const {
  return parameters_;
}

std::string Message::serialize() const {
  std::string serializedMessage = sender_ + " " + receiver_ + " " + command_;
  if (!parameters_.empty()) {
    serializedMessage += " " + parameters_;
  }
  return serializedMessage;
}

Message Message::deserialize(const std::string& serializedMessage) {
  std::istringstream iss(serializedMessage);
  std::string sender, receiver, command, parametersStr;

  iss >> sender >> receiver >> command;

  std::getline(iss, parametersStr);

  std::string parameters;

  if (!parametersStr.empty() && parametersStr[0] == ':') {
    parameters = parametersStr.substr(1);
  } else {
    if (!parametersStr.empty()) {
      parameters += ' ';
    }
    parameters += parametersStr;
  }
  return Message(sender, receiver, command, parameters);
}

}  // namespace irc
