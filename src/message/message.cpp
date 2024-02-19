#include "Message.h"

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

void Message::execute(const Message& message){

};

// std::string Message::serialize() const { }

/****
    * According to: rfc2812 / 2.3. Messages
    * Each IRC message may consist of up to three main parts: the prefix
    * (OPTIONAL), the command, and the command parameters (maximum of
    * fifteen (15)).  The prefix, command, and all parameters are separated
    * by one ASCII space character (0x20) each. 
    *
    * The presence of a prefix is indicated with a single leading ASCII
    * colon character (’:’, 0x3b), which MUST be the first character of the
    * message itself.  There MUST be NO gap (whitespace) between the colon
    * and the prefix.  The prefix is used by servers to indicate the true
    * origin of the message.  If the prefix is missing from the message, it
    * is assumed to have originated from the connection from which it was
    * received from.  Clients SHOULD NOT use a prefix when sending a
    * message; if they use one, the only valid prefix is the registered
    * nickname associated with the client.
    *
    * The command MUST either be a valid IRC command or a three (3) digit
    * number represented in ASCII text.
    *
    * IRC messages are always lines of characters terminated with a CR-LF
    * (Carriage Return - Line Feed) pair, and these messages SHALL NOT
    * exceed 512 characters in length, counting all characters including
    * the trailing CR-LF. Thus, there are 510 characters maximum allowed
    * for the command and its parameters.  There is no provision for
    * continuation of message lines.  See section 6 for more details about
    * current implementations. 
    *
    * message =  [ ":" prefix SPACE ] command [ params ] crlf
    */
Message Message::deserialize_(const std::string& serializedMessage) {
  std::istringstream iss(serializedMessage);
  std::string prefix, command;
  std::vector<std::string> parameters;

  if (serializedMessage.length() > MAX_MSG_LENGTH) {
    LOG_WARNING("Message exceeds maximum length of 512 characters, ");
    numeric_ = ERR_INPUTTOOLONG;
    return *this;

    } 
  // Sneak peek into the first char of stream for a semicolon ":"
  if (iss.peek() == ':') {
    iss.get();      // Skip the semicolon to get to the prefix itself
    iss >> prefix;  // get the prefix
    LOG_DEBUG("Got prefix: " + prefix);
  }
  iss >> command;
    LOG_DEBUG("Got command: " + command);

  std::string parameter;
  while (iss >> parameter) {
    if (parameters.size() > MESSAGE_MAX_AMOUNT_PARAMETERS)
      break;
    parameters.push_back(parameter);
    LOG_DEBUG("Got Parameter: " + parameter);
  }
  if (parameters.size() > MESSAGE_MAX_AMOUNT_PARAMETERS){
    LOG_WARNING("Too many parameters in message");
    }
  return Message(prefix, command, parameters);
}

}  // namespace irc
