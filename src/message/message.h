#ifndef MESSAGE_H
#define MESSAGE_H

#include <map>
#include <sstream>
#include <string>

namespace irc {

class Message {
 private:
  std::string sender_;
  std::string receiver_;
  std::string command_;
  std::string parameters_;

 public:
  Message(const std::string& sender, const std::string& receiver,
          const std::string& command);
  Message(const std::string& sender, const std::string& receiver,
          const std::string& command,
          const std::string& parameters);

  ~Message();

  std::string getSender() const;
  std::string getReceiver() const;
  std::string getCommand() const;
  std::string getParameters() const;

  // Serialization and Deserialization
  std::string serialize() const;
  static Message deserialize(const std::string& serializedMessage);
};

}  // namespace irc
#endif
