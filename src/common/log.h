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

#endif
