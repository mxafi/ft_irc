#ifndef MESSAGE_H
#define MESSAGE_H

#include <vector>
#include <sstream>
#include <string>

namespace irc {

class Message {
 private:
  const std::string prefix_;
  const std::string command_;
  const std::vector<std::string> parameters_;

 public:
  Message(const std::string& prefix, const std::string command,
          const std::vector<std::string> parameters);

  ~Message();

  std::string getPrefix() const;
  std::string getCommand() const;
  std::vector<std::string> getParameters() const;

  // std::string serialize() const;
  static Message deserialize(const std::string& IncomingMessage);
};

}  // namespace irc
#endif
