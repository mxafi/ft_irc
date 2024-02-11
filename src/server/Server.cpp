#include "Server.h"

irc::Server::~Server() {
  close(server_socket_fd_);
  freeaddrinfo(srvinfo_);
}

irc::Server::Server(char* port, std::string password)
    : port_(port), password_(password) {}

int irc::Server::start() {
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
  if (fcntl(server_socket_fd_, F_SETFL, O_NONBLOCK) == FCNTL_FAILURE) {
    PRINT_ERROR("server socket fcntl set nonblock failed");
    return FAILURE;
  }

  hints_.ai_family = server_socket_domain_;
  hints_.ai_socktype = server_socket_type_;
  hints_.ai_flags = AI_PASSIVE;
  if (getaddrinfo(NULL, port_, &hints_, &srvinfo_) != SUCCESS) {
    PRINT_ERROR("server getaddrinfo failed");
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
