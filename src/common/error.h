#ifndef ERROR_H
#define ERROR_H

#include <iostream>
#include "color.h"

#define PRINT_USAGE \
  { std::cerr << "Usage: ./ircserv <port> <password>" << std::endl; }

#define PRINT_ERROR(string) \
  { std::cerr << RED << "Error: " << string << RESET << std::endl; }

#define PRINT_ERROR_WITH_USAGE(string) \
  {                                    \
    PRINT_ERROR(string);               \
    PRINT_USAGE;                       \
  }

#endif
