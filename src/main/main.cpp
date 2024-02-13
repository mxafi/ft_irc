#include "main.h"  // IWYU pragma: keep // due to convention

bool isServerRunning_g = true;

void signalHandler(int signum) {
  if (signum == SIGINT) {
    LOG_INFO("server received SIGINT (Ctrl+C)");
  }
  if (signum == SIGTERM) {
    LOG_INFO("server received SIGTERM (kill)");
  }
  if (signum == SIGQUIT) {
    LOG_INFO("server received SIGQUIT (Ctrl+Backslash)");
  }
  if (signum == SIGHUP) {
    LOG_INFO("server received SIGHUP (parent died)");
  }
  isServerRunning_g = false;
}

int main(int argc, char** argv) {
  if (argc != 3) {
    LOG_ERROR("wrong number of arguments");
    PRINT_USAGE;
    return EXIT_FAILURE;
  }

  try {
    std::stoi(argv[1]);
  } catch (std::exception& e) {
    LOG_ERROR("invalid port");
    PRINT_USAGE;
    return EXIT_FAILURE;
  }
  LOG_INFO("server starting on port " << argv[1]);
  LOG_DEBUG(R"(with password ")" << argv[2] << R"(")");

  signal(SIGINT, signalHandler);   // Ctrl+C
  signal(SIGTERM, signalHandler);  // kill
  signal(SIGQUIT, signalHandler);  // Ctrl+Backslash
  signal(SIGHUP, signalHandler);   // parent dead

  irc::Server server(argv[1], argv[2]);
  if (server.start() != SUCCESS)
    return EXIT_FAILURE;
  try {
    server.loop();
  } catch (std::exception& e) {
    LOG_ERROR("Exception: " << e.what());
    return EXIT_FAILURE;
  }

  LOG_INFO("server shutting down, goodbye!");
  return EXIT_SUCCESS;
}
