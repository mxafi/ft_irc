#ifndef MAIN_H
#define MAIN_H

// disable asserts
#ifndef DEBUG
#define NDEBUG
#endif

// system definitions
#include <csignal>
#include <stdexcept>
#include <string>

// user common definitions
#include "../common/log.h"
#include "../common/magicNumber.h"

// user module definitions
#include "../server/Server.h"

#endif
