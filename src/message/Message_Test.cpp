#include "Message.h"

/**************************************************************************************************/
/* Test Valid message with Prefix and  Parameters
/**************************************************************************************************/
TEST(MessageDeserialization, ValidMessageWithPrefixAndParameters) {
  std::string serializedMessage =
      "nick!~user@host.com PRIVMSG #channel :Hello, World!";
  Message message = Message::deserialize(serializedMessage);

  EXPECT_EQ(message.getPrefix(), "nick!~user@host.com");
  EXPECT_EQ(message.getCommand(), "PRIVMSG");
  EXPECT_EQ(message.getParameters().size(), 2);
  EXPECT_EQ(message.getParameters()[0], "#channel");
  EXPECT_EQ(message.getParameters()[1], "Hello, World!");
}
/**************************************************************************************************/
/* TODO:
 * - Valid message with no prefix and max amount of parameters
 * - Message with prefix and max amount of parameters
/**************************************************************************************************/

/**************************************************************************************************/
/* Test prefix starts with a colon ":"
/**************************************************************************************************/
TEST(MessageDeserialization, PrefixContainsSemicolon) {
    std::string serializedMessage = ":prefix COMMAND parameter";
    Message message = Message::deserialize(serializedMessage);

    ASSERT_TRUE(message.getPrefix().find(':') != std::string::npos);
}

