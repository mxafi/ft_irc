#include "Server.h"

irc::Server::Server(int port, std::string password)
    : port_(port), password_(password) {}

void irc::Server::start() {}

void irc::Server::loop() {}
