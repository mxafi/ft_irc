#ifndef MESSAGE_H
#define MESSAGE_H

#include <sstream>
#include <string>
#include <vector>
#include "../common/log.h"
#include "../common/magicNumber.h"

namespace irc {

class Message {
 private:
  std::string prefix_;
  std::string command_;
  std::vector<std::string> parameters_;
  int numeric_;
  void deserialize_(const std::string& serializedMessage);
  void setPrefix_(std::istringstream& serializedStream);
  void setCommand_(std::istringstream& serializedMessage);
  void setParameters_(std::istringstream& serializedStream);
  Message(const Message& original) {};
  Message operator=(const Message& original) {};

  void checkNulChar(const std::string& serializedMessage);
  void checkMessageLength(const std::string& serializedMessage);

 public:
  Message(std::string serializedMessage);

  ~Message();

  std::string getPrefix() const;
  std::string getCommand() const;
  std::vector<std::string> getParameters() const;

  // std::string serialize() const;

  void execute(const Message& message);
};

}  // namespace irc
#endif
