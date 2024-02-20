#include "Message.h"

namespace irc {
/****
    * Servers and clients send each other messages, which may or may not
    * generate a reply.  If the message contains a valid command, the 
    * client should expect a reply as specified but it is not advised to wait
    * forever for the reply; client to server and server to server 
    * communication is essentially asynchronous by nature.
    *
    * The message object constructor is parsing the IRC received messages
    * and initializes its prefix (OPTIONAL), command and parameters.
    */
Message::Message(const std::string serializedMessage) {
  checkNulChar(serializedMessage);
  checkMessageLength(serializedMessage);
  deserialize_(serializedMessage);
}

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

/****
    * The NUL (%x00) character is not special in message framing, and
    * basically could end up inside a parameter, but it would cause
    * extra complexities in normal C string handling. Therefore, NUL
    * is not allowed within messages.
    */
void Message::checkNulChar(const std::string &serializedMessage) {
    if (serializedMessage.find('\0') != std::string::npos)
        LOG_WARNING("Message contains illegal NUL Character");
        numeric_ = ERR_CUSTOM_ILLEGALNUL;
}


/****
    * IRC messages are always lines of characters terminated with a CR-LF
    * (Carriage Return - Line Feed) pair, and these messages SHALL NOT
    * exceed 512 characters in length, counting all characters including
    * the trailing CR-LF. Thus, there are 510 characters maximum allowed
    * for the command and its parameters.  There is no provision for
    * continuation of message lines. 
    */
void Message::checkMessageLength(const std::string& serializedMessage) {
  if (serializedMessage.length() > MAX_MSG_LENGTH) {
    LOG_WARNING("Input line was too long, " << serializedMessage.length() << "instead of 512");
    numeric_ = ERR_INPUTTOOLONG;
  }
}

/****
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
    * prefix =  servername / ( nickname [ [ "!" user ] "@" host ] )
    */
void Message::setPrefix_(std::istringstream& serializedStream) {
  // Sneak peek into the first char of stream for a semicolon ":"
  if (serializedStream.peek() == ':') {
    serializedStream.get();  // Skip the semicolon to get to the prefix_ itself
    serializedStream >> prefix_;  // get the prefix_
    LOG_DEBUG("Got prefix_: " + prefix_);
  }
}

/****
    * The command MUST either be a valid IRC command or a three (3) digit
    * number represented in ASCII text.
    * 
    * command =  1*letter / 3digit
    */
void Message::setCommand_(std::istringstream& serializedMessage) {
  serializedMessage >> command_;
  LOG_DEBUG("Got command_: " + command_);
}

/****
    * The command parameters constitute the remaining part of the message and 
    * should amount to 15 maximum separated by ASCII space characters (0x20) each.
    *
    * params =  *14( SPACE middle ) [ SPACE ":" trailing ]
    *        =/ 14( SPACE middle ) [ SPACE [ ":" ] trailing ]
    */
void Message::setParameters_(std::istringstream& serializedMessage) {
  std::string parameter;
  while (serializedMessage >> parameter) {
    if (parameters_.size() > MESSAGE_MAX_AMOUNT_PARAMETERS)
      break;
    parameters_.push_back(parameter);
    LOG_DEBUG("Got Parameter: " + parameter);
  }
  if (parameters_.size() > MESSAGE_MAX_AMOUNT_PARAMETERS) {
    LOG_WARNING("Too many parameters in message");
    numeric_ = ERR_CUSTOM_TOOMANYPARAMS;
  }
}

/****
    * According to: rfc2812 / 2.3. Messages
    * Each IRC message may consist of up to three main parts: the prefix
    * (OPTIONAL), the command, and the command parameters (maximum of
    * fifteen (15)).  The prefix, command, and all parameters are separated
    * by one ASCII space character (0x20) each. 
    *
    * message    =  [ ":" prefix SPACE ] command [ params ] crlf
    * prefix     =  servername / ( nickname [ [ "!" user ] "@" host ] )
    * command    =  1*letter / 3digit
    * params     =  *14( SPACE middle ) [ SPACE ":" trailing ]
    *            =/ 14( SPACE middle ) [ SPACE [ ":" ] trailing ]
    * nospcrlfcl =  %x01-09 / %x0B-0C / %x0E-1F / %x21-39 / %x3B-FF
    *               ; any octet except NUL, CR, LF, " " and ":"
    * middle     =  nospcrlfcl *( ":" / nospcrlfcl )*
    * trailing   =  *( ":" / " " / nospcrlfcl )
    * SPACE      =  %x20        ; space character
    * crlf       =  %x0D %x0A   ; "carriage return" "linefeed"
    */
void Message::deserialize_(const std::string& serializedMessage) {
  std::istringstream iss(serializedMessage);

  setPrefix_(iss);
  setCommand_(iss);
  setParameters_(iss);
}

// std::string Message::serialize() const { }
}  // namespace irc
