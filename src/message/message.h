#ifndef MESSAGE_H
#define MESSAGE_H

#include <sstream>
#include <string>
#include <vector>
#include "../common/magicNumber.h"
#include "../common/log.h"

namespace irc {

class Message {
 private:
  std::string prefix_;
  std::string command_;
  std::vector<std::string> parameters_;
  int numeric_;
  Message deserialize_(const std::string& IncomingMessage);

 public:
  Message(const std::string& prefix, const std::string command,
          const std::vector<std::string> parameters);

  ~Message();

  std::string getPrefix() const;
  std::string getCommand() const;
  std::vector<std::string> getParameters() const;

  // std::string serialize() const;

  void execute(const Message &message);
};

}  // namespace irc
#endif
