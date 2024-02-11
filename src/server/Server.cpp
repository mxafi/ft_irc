#include "Server.h"

irc::Server::~Server() {
  close(server_socket_fd_);
  freeaddrinfo(srvinfo_);
}

irc::Server::Server(char* port, std::string password)
    : port_(port), password_(password) {}

int irc::Server::start() {
  memset(&hints_, 0, sizeof hints_);
  hints_.ai_family = AF_INET;
  hints_.ai_socktype = SOCK_STREAM;
  hints_.ai_flags = AI_PASSIVE;
  if (int gai_ret = getaddrinfo(NULL, port_, &hints_, &srvinfo_) != SUCCESS) {
    PRINT_ERROR("server getaddrinfo failed: (" << gai_ret << ") " << gai_strerror(gai_ret));
    return FAILURE;
  }

  server_socket_fd_ = socket(server_socket_domain_, server_socket_type_,
                             server_socket_protocol_);
  if (server_socket_fd_ == SOCKET_FAILURE) {
    PRINT_ERROR("server socket creation failed");
    return FAILURE;
  }

  int fcntl_flags = fcntl(server_socket_fd_, F_GETFL);
  if (fcntl_flags == FCNTL_FAILURE) {
    PRINT_ERROR("server socket fcntl get flags failed");
    return FAILURE;
  }
  if (fcntl(server_socket_fd_, F_SETFL, fcntl_flags | O_NONBLOCK) == FCNTL_FAILURE) {
    PRINT_ERROR("server socket fcntl set nonblock failed");
    return FAILURE;
  }

  if (bind(server_socket_fd_, srvinfo_->ai_addr, srvinfo_->ai_addrlen) ==
      BIND_FAILURE) {
    PRINT_ERROR("server bind failed");
    return FAILURE;
  }

  if (listen(server_socket_fd_, SOMAXCONN) == LISTEN_FAILURE) {
    PRINT_ERROR("server listen failed");
    return FAILURE;
  }

  return SUCCESS;
}

void irc::Server::loop() {}
