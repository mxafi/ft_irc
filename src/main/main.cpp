#include "main.h"  // IWYU pragma: keep // due to convention

int main(int argc, char** argv) {
  if (argc != 3) {
    PRINT_ERROR_WITH_USAGE("wrong number of arguments");
    return EXIT_FAILURE;
  }

  try {
    std::stoi(argv[1]);
  } catch (std::exception& e) {
    PRINT_ERROR_WITH_USAGE("invalid port");
    return EXIT_FAILURE;
  }

  irc::Server server(argv[1], argv[2]);
  if (server.start() != SUCCESS)
    return EXIT_FAILURE;
  try {
    server.loop();
  } catch (std::exception& e) {
    PRINT_ERROR("Exception: " << e.what());
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
