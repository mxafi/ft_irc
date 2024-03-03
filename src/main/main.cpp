/**
 * @file main.cpp
 * @brief This file contains the main function for the IRC server.
 * 
 * The main function initializes the server, handles signals, and starts the server loop.
 * It also performs argument validation and error handling.
 * 
 * The server can be started by providing two command-line arguments: the port number and the password.
 * 
 * Supported signals:
 * - SIGINT:   server received SIGINT (Ctrl+C)
 * - SIGTERM:  server received SIGTERM (kill)
 * - SIGQUIT:  server received SIGQUIT (Ctrl+Backslash)
 * - SIGHUP:   server received SIGHUP (parent died)
 * 
 * If the server encounters an unhandled exception during execution, it logs the exception and exits with failure status.
 * 
 * @param argc The number of command-line arguments.
 * @param argv An array of command-line arguments.
 * @return int The exit status of the program.
 */

#include "main.h"  // IWYU pragma: keep // due to convention

/**
 * @brief Indicates whether the server is currently running or not.
 */
bool isServerRunning_g = false;

/**
 * @brief The hostname of the server.
 */
std::string serverHostname_g;

/**
 * @brief Signal handler function that handles all signals caught by the program.
 * 
 * @param signum The signal number.
 */
void signalHandler(int signum) {
  if (signum == SIGINT) {
    LOG_INFO("Server received SIGINT (Ctrl+C)");
  }
  if (signum == SIGTERM) {
    LOG_INFO("Server received SIGTERM (kill)");
  }
  if (signum == SIGQUIT) {
    LOG_INFO("Server received SIGQUIT (Ctrl+Backslash)");
  }
  if (signum == SIGHUP) {
    LOG_INFO("Server received SIGHUP (parent died)");
  }
  isServerRunning_g = false;
}

/**
 * @brief The entry point of the program.
 *
 * This function is the starting point of the program. It parses the command line arguments,
 * initializes the server, and starts the server loop. It also handles signals for graceful shutdown.
 *
 * @param argc The number of command line arguments.
 * @param argv An array of command line arguments.
 * @return int The exit status of the program.
 */
int main(int argc, char** argv) {
  if (argc != 3) {
    LOG_ERROR("main: wrong number of arguments");
    PRINT_USAGE;
    return EXIT_FAILURE;
  }

  try {
    std::stoi(argv[1]);
  } catch (std::exception& e) {
    LOG_ERROR("main: invalid port");
    PRINT_USAGE;
    return EXIT_FAILURE;
  }
  LOG_INFO("Server starting on port " << argv[1]);
  LOG_DEBUG("with password \")" << argv[2] << "\"");

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
    LOG_ERROR("main: Exception: " << e.what());
    return EXIT_FAILURE;
  }

  LOG_INFO("Server shutting down, goodbye!");
  return EXIT_SUCCESS;
}
