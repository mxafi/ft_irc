#include "Server.h"

namespace irc {

Server::~Server() {
  std::map<int, Client>::iterator it = clients_.begin();
  while (it != clients_.end()) {
    close(it->first);
    it++;
  }
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
  LOG_DEBUG("server socket creation success on fd: " << server_socket_fd_);

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

  if (ON_MACOS) {
    int optval = TRUE;
    if (setsockopt(server_socket_fd_, SOL_SOCKET, SO_NOSIGPIPE, &optval,
                   sizeof(optval)) == SETSOCKOPT_FAILURE) {
      LOG_ERROR("server socket setsockopt failed: " << strerror(errno));
      return FAILURE;
    }
    LOG_DEBUG("server socket setsockopt success");
  }

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

  isServerRunning_g = true;
  start_time_ = time(nullptr);
  return SUCCESS;
}

void irc::Server::loop() {
  std::vector<pollfd> pollfds;
  pollfd server_pollfd;
  server_pollfd.fd = server_socket_fd_;
  server_pollfd.events = POLLIN | POLLERR;
  pollfds.push_back(server_pollfd);

  LOG_DEBUG("server loop start")
  while (isServerRunning_g) {
    std::vector<pollfd> tmp_pollfds;

    if (poll(pollfds.data(), static_cast<unsigned int>(pollfds.size()), -1) ==
        POLL_FAILURE) {
      if (errno == EINTR && isServerRunning_g == false) {
        continue;
      }
      throw std::runtime_error("server poll failed");
    }

    std::vector<pollfd>::iterator it = pollfds.begin();
    while (it != pollfds.end()) {
      if (it->revents & POLLIN) {
        if (it->fd == server_socket_fd_) {
          if (acceptClient_(tmp_pollfds) == ACCEPT_FAILURE) {
            continue;
          }
        } else {
          try {
            Client& client = clients_.at(it->fd);
            recvToBuffer_(client);
            if (client.getWantDisconnect() == true) {
              disconnectClient_(pollfds, it);
              break;
            }
            std::string messageString;
            while (extractMessageString_(messageString, client) != FAILURE) {
              Message message(messageString);
              if (message.getNumeric() != SUCCESS) {
                LOG_DEBUG("server got malformed message from client on fd "
                          << client.getFd() << ": " << messageString);
                handleMalformedMessage_(client, message);
                continue;
              }
              LOG_DEBUG("server received message from client on fd "
                        << client.getFd() << ": " << messageString);
              // call CommandHandler here with parameters: clients_, client, message
            }
          } catch (std::out_of_range& e) {
            LOG_ERROR("server client object not found for POLLIN at fd "
                      << it->fd << ": " << e.what());
            disconnectClient_(pollfds, it);
            break;
          }
        }
      }
      if (it->revents & POLLOUT) {
        try {
          Client& client = clients_.at(it->fd);
          sendFromBuffer_(client);  //logs nothing if sendbuffer is empty
          if (client.getWantDisconnect() == true) {
            disconnectClient_(pollfds, it);
            break;
          }
        } catch (std::out_of_range& e) {
          LOG_ERROR("server client object not found for POLLOUT at fd "
                    << it->fd << ": " << e.what());
          disconnectClient_(pollfds, it);
          break;
        }
      }
      if (it->revents & POLLERR) {
        if (it->fd == server_socket_fd_) {
          throw std::runtime_error("server socket pollerr");
        } else {
          disconnectClient_(pollfds, it);
          break;
        }
      }
      it++;
    }

    pollfds.insert(pollfds.end(), tmp_pollfds.begin(), tmp_pollfds.end());
  }
  LOG_DEBUG("server loop end")
}

int Server::acceptClient_(std::vector<pollfd>& pollfds) {
  pollfd client_pollfd;
  struct sockaddr client_info;
  unsigned int client_info_length = sizeof client_info;
  int new_client_fd =
      accept(server_socket_fd_, &client_info, &client_info_length);
  if (new_client_fd == ACCEPT_FAILURE) {
    LOG_ERROR(
        "server failed to accept new client connection: " << strerror(errno));
    return ACCEPT_FAILURE;
  }
  clients_.insert(
      std::make_pair(new_client_fd, Client(new_client_fd, client_info)));
  client_pollfd.fd = new_client_fd;
  client_pollfd.events = POLLIN | POLLOUT | POLLERR;
  pollfds.push_back(client_pollfd);
  LOG_INFO("New client connection on fd " << new_client_fd);
  LOG_INFO("Clients on server now: " << clients_.size());
  return new_client_fd;
}

int Server::disconnectClient_(std::vector<pollfd>& poll_fds,
                              std::vector<pollfd>::iterator& it) {
  int client_fd = it->fd;
  LOG_DEBUG("server disconnecting client on fd " << client_fd);
  close(client_fd);
  unsigned long clients_erased = clients_.erase(client_fd);
  if (clients_erased == 0) {
    LOG_WARNING("server client not found at fd "
                << client_fd << " to erase from clients_ map");
  }
  poll_fds.erase(it);
  LOG_INFO("Clients remaining on server: " << clients_.size());
  return SUCCESS;
}

long long Server::sendFromBuffer_(Client& client) {
  std::string& buffer = client.getSendBuffer();
  if (buffer.empty()) {
    return SUCCESS;
  }
  long long send_ret =
      send(client.getFd(), buffer.c_str(), buffer.size(), MSG_NOSIGNAL);
  if (send_ret == SEND_FAILURE) {
    LOG_ERROR("server send failed: " << strerror(errno));
    return SEND_FAILURE;
  }
  LOG_DEBUG("server sent " << send_ret << " bytes to client on fd "
                           << client.getFd());
  buffer.erase(0, static_cast<unsigned long>(send_ret));
  return send_ret;
}

long long Server::recvToBuffer_(Client& client) {
  char tmpRecvBuffer[SERVER_RECV_BUFFER_SIZE];
  long long recv_ret = RECV_FAILURE;
  std::string& buf = client.getRecvBuffer();

  memset(tmpRecvBuffer, 0, SERVER_RECV_BUFFER_SIZE);
  recv_ret = recv(client.getFd(), tmpRecvBuffer, SERVER_RECV_BUFFER_SIZE, 0);
  if (recv_ret == RECV_FAILURE) {
    LOG_ERROR("server recv failed on fd: " << client.getFd()
                                           << " with: " << strerror(errno));
    return RECV_FAILURE;
  }
  if (recv_ret == RECV_ORDERLY_SHUTDOWN) {
    LOG_INFO("Client on fd " << client.getFd() << " disconnected gracefully");
    client.setWantDisconnect();
    return RECV_ORDERLY_SHUTDOWN;
  }
  LOG_DEBUG("server received a packet of "
            << recv_ret << " bytes from client on fd " << client.getFd());
  buf.append(tmpRecvBuffer, static_cast<unsigned long>(recv_ret));
  return recv_ret;
}

/**
 * @brief Extracts a message string from the client's recv buffer
 * 
 * @param message The message string to be extracted
 * @param client The client whose recv buffer is to be used
 * @return int SUCCESS if message was extracted, FAILURE if not
 */
int Server::extractMessageString_(std::string& message, Client& client) {
  std::string& buf = client.getRecvBuffer();
  std::string pattern = "\r\n";
  std::string::size_type pos = buf.find(pattern);
  if (pos == std::string::npos) {
    return FAILURE;
  }
  message.clear();
  message = buf.substr(0, pos + pattern.length());
  buf.erase(0, pos + pattern.length());
  LOG_DEBUG("server extracted message::" << message << ":: from client on fd "
                                         << client.getFd());

  return SUCCESS;
}

void Server::handleMalformedMessage_(Client& client, Message& message) {
  int numeric = message.getNumeric();
  if (numeric == ERR_CUSTOM_ILLEGALNUL) {
    LOG_WARNING(
        "Client on fd "
        << client.getFd()
        << " sent a message that contained a null character: ignoring it");
  } else if (numeric == ERR_INPUTTOOLONG) {
    LOG_WARNING("Client on fd "
                << client.getFd()
                << " sent a message that was too long: ignoring it");
  } else if (numeric == ERR_CUSTOM_TOOMANYPARAMS) {
    LOG_WARNING("Client on fd "
                << client.getFd()
                << " sent a message with too many parameters: ignoring it");
  }
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

std::string Server::getStartTimeString() {
  return std::string(ctime(&start_time_));
}

}  // namespace irc
