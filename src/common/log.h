#ifndef LOG_H
#define LOG_H

#include <iostream>
#include "color.h"

#ifdef DEBUG
#define LOG_DEBUG(string) \
  { std::cout << CYAN << "Debug: " << string << RESET << std::endl; }
#else
#define LOG_DEBUG(string)
#endif

#define LOG_INFO(string) \
  { std::cout << WHITE << "Info: " << string << RESET << std::endl; }

#define LOG_WARNING(string) \
  { std::cout << YELLOW << "Warning: " << string << RESET << std::endl; }

#define LOG_ERROR(string) \
  { std::cerr << RED << "Error: " << string << RESET << std::endl; }

#define PRINT_USAGE \
  { std::cout << "Usage: ./ircserv <port> <password>" << std::endl; }

#endif
