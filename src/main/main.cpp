#include "main.h"  // IWYU pragma: keep // due to convention
#include <sys/signal.h>
#include <cmath>

bool isServerRunning_g = true;

void signalHandler(int signum) {
  if (signum == SIGINT) {
    LOG_INFO("server received SIGINT");
    isServerRunning_g = false;
  }
}

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
  LOG_INFO("server starting on port " << argv[1]);
  LOG_DEBUG(R"(with password ")" << argv[2] << R"(")");

  signal(SIGINT, signalHandler);
  irc::Server server(argv[1], argv[2]);
  if (server.start() != SUCCESS)
    return EXIT_FAILURE;
  try {
    server.loop();
  } catch (std::exception& e) {
    PRINT_ERROR("Exception: " << e.what());
    return EXIT_FAILURE;
  }

  LOG_INFO("server shutting down, goodbye!");
  return EXIT_SUCCESS;
}
