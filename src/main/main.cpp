#include "main.h"

int main(int argc, char** argv) {
  if (argc != 3) {
    PRINT_ERROR_WITH_USAGE("wrong number of arguments");
    return EXIT_FAILURE;
  }

  int port;
  try {
    port = std::stoi(argv[1]);
  } catch (std::exception& e) {
    PRINT_ERROR_WITH_USAGE("invalid port");
    return EXIT_FAILURE;
  }

  irc::Server server(port, argv[2]);
  server.start();
  try {
    server.loop();
  } catch (std::exception& e) {
    PRINT_ERROR("Exception: " << e.what());
  }

  return EXIT_SUCCESS;
}
