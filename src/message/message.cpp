#include "Message.h"

/****
    * According to: rfc2812 / 2.3. Messages
    * Each IRC message may consist of up to three main parts: the prefix
    * (OPTIONAL), the command, and the command parameters (maximum of
    * fifteen (15)).  The prefix, command, and all parameters are separated
    * by one ASCII space character (0x20) each. 
    */
namespace irc {

Message::Message(const std::string& prefix, const std::string command,
                 const std::vector<std::string> parameters)
    : prefix_(prefix), command_(command), parameters_(parameters){};

Message::~Message(){};

std::string Message::getPrefix() const {
  return prefix_;
}

std::string Message::getCommand() const {
  return command_;
}

std::vector<std::string> Message::getParameters() const {
  return parameters_;
}

// std::string Message::serialize() const { }

Message Message::deserialize(const std::string& serializedMessage) {
  std::istringstream iss(serializedMessage);
  std::string prefix, command;
  std::vector<std::string> parameters;

  iss >> prefix >> command;

  std::string parameter;
  while (iss >> parameter) {
    parameters.push_back(parameter);
  }
  return Message(prefix, command, parameters);
}

}  // namespace irc
