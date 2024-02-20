#include "Server.h"

namespace irc {

Server::~Server() {
  close(server_socket_fd_);
  freeaddrinfo(srvinfo_);
}

Server::Server(char* port, std::string password)
    : port_(port), password_(password) {}

int Server::setServerHostname_() {
  int hostnameLength;
  char hostname[HOSTNAME_MAX_LENGTH];
  hostnameLength = gethostname(hostname, HOSTNAME_MAX_LENGTH);

  if (hostnameLength == GETHOSTNAME_FAILURE) {
    return FAILURE;
  }

  if (hostnameLength >= HOSTNAME_MAX_LENGTH) {
    LOG_WARNING(
        "server failed to get hostname: hostname too long, using hostaddress "
        "instead");
    if (inet_ntop(server_socket_domain_,
                  &((struct sockaddr_in*)srvinfo_->ai_addr)->sin_addr, hostname,
                  HOSTNAME_MAX_LENGTH) == NULL) {
      LOG_ERROR("server inet_ntop failed: " << strerror(errno));
      return FAILURE;
    }
  }

  serverHostname_g = std::string(hostname);
  LOG_INFO("with hostname " << serverHostname_g);
  return SUCCESS;
}

int Server::start() {
  memset(&hints_, 0, sizeof hints_);
  hints_.ai_family = server_socket_domain_;
  hints_.ai_socktype = server_socket_type_;
  hints_.ai_flags = AI_PASSIVE;
  if (int gai_ret = getaddrinfo(NULL, port_, &hints_, &srvinfo_) != SUCCESS) {
    LOG_ERROR("server getaddrinfo failed: (" << gai_ret << ") "
                                             << gai_strerror(gai_ret));
    return FAILURE;
  }
  LOG_DEBUG("server getaddrinfo success");

  if (setServerHostname_() == FAILURE) {
    LOG_ERROR("server hostname fetching failed");
    return FAILURE;
  }

  server_socket_fd_ = socket(srvinfo_->ai_family, srvinfo_->ai_socktype,
                             server_socket_protocol_);
  if (server_socket_fd_ == SOCKET_FAILURE) {
    LOG_ERROR("server socket creation failed");
    return FAILURE;
  }
  LOG_DEBUG("server socket creation success");

  int fcntl_flags = fcntl(server_socket_fd_, F_GETFL);
  if (fcntl_flags == FCNTL_FAILURE) {
    LOG_ERROR("server socket fcntl get flags failed");
    return FAILURE;
  }
  if (fcntl(server_socket_fd_, F_SETFL, fcntl_flags | O_NONBLOCK) ==
      FCNTL_FAILURE) {
    LOG_ERROR("server socket fcntl set nonblock failed");
    return FAILURE;
  }
  LOG_DEBUG("server socket fcntl nonblock success");

  if (bind(server_socket_fd_, srvinfo_->ai_addr, srvinfo_->ai_addrlen) ==
      BIND_FAILURE) {
    LOG_ERROR("server bind failed");
    return FAILURE;
  }
  LOG_DEBUG("server bind success");

  if (listen(server_socket_fd_, SOMAXCONN) == LISTEN_FAILURE) {
    LOG_ERROR("server listen failed");
    return FAILURE;
  }
  LOG_DEBUG("server listen success");

  return SUCCESS;
}

void irc::Server::loop() {
  std::vector<pollfd> pollfds;
  pollfd server_pollfd;
  server_pollfd.fd = server_socket_fd_;
  server_pollfd.events = POLLIN;
  pollfds.push_back(server_pollfd);

  LOG_DEBUG("server loop start")
  while (isServerRunning_g) {
    if (poll(pollfds.data(), static_cast<unsigned int>(pollfds.size()), -1) ==
        POLL_FAILURE) {
      if (errno == EINTR && isServerRunning_g == false) {
        continue;
      }
      throw std::runtime_error("server poll failed");
    }

    std::vector<pollfd>::iterator it = pollfds.begin();
    while (it != pollfds.end()) {
      if (it->revents & POLLIN) {  // ready to recv()
        if (it->fd == server_socket_fd_) {
          // handle incoming new client connection
        } else {
          // handle incoming request from existing client connection
        }
      }
      if (it->revents & POLLOUT) {  // ready to send()
        // handle outgoing response to existing client connection
      }
      if (it->revents & POLLERR) {  // socket disconnect or error
        // handle client socket disconnect or server socket error
      }
      it++;
    }
  }
  LOG_DEBUG("server loop end")
}

// Getter functions

char* Server::getPort() {
  return port_;
}

std::string Server::getPassword() {
  return password_;
}

int Server::getServerSocketFd() {
  return server_socket_fd_;
}

int Server::getServerSocketDomain() {
  return server_socket_domain_;
}

int Server::getServerSocketType() {
  return server_socket_type_;
}

int Server::getServerSocketProtocol() {
  return server_socket_protocol_;
}

struct addrinfo& Server::getServerInfo() {
  return *srvinfo_;
}

}  // namespace irc
