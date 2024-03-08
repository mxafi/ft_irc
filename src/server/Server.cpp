/**
 * @file Server.cpp
 * @brief Implementation file for the Server class.
 * 
 * This file contains the implementation of the Server class, which represents an IRC server.
 * It includes the definitions of the constructor, destructor, and member functions of the Server class.
 * 
 * The Server class provides functionality to start the server, accept client connections, handle client messages,
 * send messages to clients, and handle client disconnections.
 * 
 * The Server class also includes private helper functions for setting the server hostname, accepting client connections,
 * receiving messages from clients, sending messages to clients, and disconnecting clients.
 * 
 * @note This file should be included in the compilation of the IRC server application.
 */

#include "Server.h"

namespace irc {

/**
 * @brief Destructor for the Server class.
 * 
 * This destructor closes all client sockets and the server socket,
 * and frees the address information structure.
 */
Server::~Server() {
  std::map<int, Client>::iterator it = clients_.begin();
  while (it != clients_.end()) {
    close(it->first);
    it++;
  }
  close(server_socket_fd_);
  freeaddrinfo(srvinfo_);
}

/**
 * @brief Constructs a Server object with the specified port and password.
 * 
 * @param port The port number to bind the server to.
 * @param password The password required to connect to the server.
 */
Server::Server(char* port, std::string password)
    : port_(port), password_(password) {}

/**
 * @brief Sets the server hostname.
 * 
 * This function retrieves the hostname of the server and stores it in the `serverHostname_g` variable.
 * If the hostname is too long, it falls back to using the host address instead.
 * 
 * @return Returns SUCCESS if the hostname is successfully set, otherwise returns FAILURE.
 */
int Server::setServerHostname_() {
  int hostnameLength;
  char hostname[HOSTNAME_MAX_LENGTH];
  hostnameLength = gethostname(hostname, HOSTNAME_MAX_LENGTH);

  if (hostnameLength == GETHOSTNAME_FAILURE) {
    return FAILURE;
  }

  if (hostnameLength >= HOSTNAME_MAX_LENGTH) {
    LOG_WARNING(
        "Server::setServerHostname_: failed to get hostname: hostname too "
        "long, using hostaddress instead");
    if (inet_ntop(server_socket_domain_,
                  &((struct sockaddr_in*)srvinfo_->ai_addr)->sin_addr, hostname,
                  HOSTNAME_MAX_LENGTH) == NULL) {
      LOG_ERROR(
          "Server::setServerHostname_: inet_ntop failed: " << strerror(errno));
      return FAILURE;
    }
  }

  serverHostname_g = std::string(hostname);
  LOG_INFO("with hostname " << serverHostname_g);
  return SUCCESS;
}

/**
 * @brief Starts the server.
 * 
 * This function initializes and starts the server by performing the following steps:
 * 1. Calls getaddrinfo() to retrieve address information for the server.
 * 2. Sets the server hostname.
 * 3. Creates a socket using socket().
 * 4. Sets the socket to non-blocking mode using fcntl().
 * 5. Sets the SO_NOSIGPIPE option on the socket using setsockopt() if compiling on MacOS.
 * 6. Binds the socket to the server address using bind().
 * 7. Starts listening for incoming connections using listen().
 * 8. Sets the isServerRunning_g flag to true.
 * 9. Records the start time of the server.
 * 
 * @return Returns SUCCESS if the server starts successfully, otherwise returns FAILURE.
 */
int Server::start() {
  memset(&hints_, 0, sizeof hints_);
  hints_.ai_family = server_socket_domain_;
  hints_.ai_socktype = server_socket_type_;
  hints_.ai_flags = AI_PASSIVE;
  if (int gai_ret = getaddrinfo(NULL, port_, &hints_, &srvinfo_) != SUCCESS) {
    LOG_ERROR("Server::start: getaddrinfo failed: (" << gai_ret << ") "
                                                     << gai_strerror(gai_ret));
    return FAILURE;
  }
  LOG_DEBUG("Server::start: getaddrinfo success");

  if (setServerHostname_() == FAILURE) {
    LOG_ERROR("Server::start: hostname fetching failed");
    return FAILURE;
  }

  server_socket_fd_ = socket(srvinfo_->ai_family, srvinfo_->ai_socktype,
                             server_socket_protocol_);
  if (server_socket_fd_ == SOCKET_FAILURE) {
    LOG_ERROR("Server::start: socket creation failed");
    return FAILURE;
  }
  LOG_DEBUG("server socket creation success on fd: " << server_socket_fd_);

  int fcntl_flags = fcntl(server_socket_fd_, F_GETFL);
  if (fcntl_flags == FCNTL_FAILURE) {
    LOG_ERROR("Server::start: socket fcntl get flags failed");
    return FAILURE;
  }
  if (fcntl(server_socket_fd_, F_SETFL, fcntl_flags | O_NONBLOCK) ==
      FCNTL_FAILURE) {
    LOG_ERROR("Server::start: socket fcntl set nonblock failed");
    return FAILURE;
  }
  LOG_DEBUG("Server::start: socket fcntl nonblock success");

  if (ON_MACOS) {
    int optval = TRUE;
    if (setsockopt(server_socket_fd_, SOL_SOCKET, SO_NOSIGPIPE, &optval,
                   sizeof(optval)) == SETSOCKOPT_FAILURE) {
      LOG_ERROR("Server::start: socket setsockopt failed: " << strerror(errno));
      return FAILURE;
    }
    LOG_DEBUG("Server::start: socket setsockopt success");
  }

  if (bind(server_socket_fd_, srvinfo_->ai_addr, srvinfo_->ai_addrlen) ==
      BIND_FAILURE) {
    LOG_ERROR("Server::start: bind failed");
    return FAILURE;
  }
  LOG_DEBUG("Server::start: bind success");

  if (listen(server_socket_fd_, SOMAXCONN) == LISTEN_FAILURE) {
    LOG_ERROR("Server::start: listen failed");
    return FAILURE;
  }
  LOG_DEBUG("Server::start: listen success");

  isServerRunning_g = true;
  start_time_ = time(nullptr);
  return SUCCESS;
}

/**
 * @brief Executes the main loop of the IRC server.
 * 
 * This function continuously polls for events on the server socket and client sockets,
 * and handles the corresponding actions based on the received events.
 * 
 * @details The loop runs until the server is stopped by setting the `isServerRunning_g` flag to false.
 * 
 * @throws std::runtime_error if the server poll fails.
 * 
 * @note This function is responsible for accepting new client connections, receiving and handling messages
 * from clients, sending messages to clients, and handling disconnections.
 * 
 * @note The function also logs debug messages for various events during the server loop.
 */
void Server::loop() {
  std::vector<pollfd> pollfds;
  pollfd server_pollfd;
  server_pollfd.fd = server_socket_fd_;
  server_pollfd.events = POLLIN | POLLERR;
  pollfds.push_back(server_pollfd);

  LOG_DEBUG("Server::loop: loop start")
  while (isServerRunning_g) {
    std::vector<pollfd> tmp_pollfds;

    if (poll(pollfds.data(), static_cast<unsigned int>(pollfds.size()), -1) ==
        POLL_FAILURE) {
      if (errno == EINTR && isServerRunning_g == false) {
        continue;
      }
      throw std::runtime_error("Server::loop: poll failed");
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
                LOG_DEBUG(
                    "Server::loop: got malformed message from client on fd "
                    << client.getFd() << ": " << messageString);
                handleMalformedMessage_(client, message);
                continue;
              }
              LOG_DEBUG("Server::loop: received message from client on fd "
                        << client.getFd() << ": " << messageString);
              Command command(message, client, clients_, password_,
                              start_time_, channels_);
            }
          } catch (std::out_of_range& e) {
            LOG_ERROR("Server::loop: out of range exception for fd "
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
          LOG_ERROR("Server::loop: client object not found for POLLOUT at fd "
                    << it->fd << ": " << e.what());
          disconnectClient_(pollfds, it);
          break;
        }
      }
      if (it->revents & POLLERR) {
        if (it->fd == server_socket_fd_) {
          throw std::runtime_error("Server::loop: socket pollerr");
        } else {
          disconnectClient_(pollfds, it);
          break;
        }
      }
      it++;
    }

    pollfds.insert(pollfds.end(), tmp_pollfds.begin(), tmp_pollfds.end());
  }
  LOG_DEBUG("Server::loop: loop end")
}

/**
 * Accepts a new client connection and adds it to the server's list of clients.
 * 
 * @param pollfds A reference to the vector of pollfds used for polling events.
 * @return The file descriptor of the newly accepted client, or ACCEPT_FAILURE if an error occurred.
 */
int Server::acceptClient_(std::vector<pollfd>& pollfds) {
  pollfd client_pollfd;
  struct sockaddr client_info;
  unsigned int client_info_length = sizeof client_info;
  int new_client_fd =
      accept(server_socket_fd_, &client_info, &client_info_length);
  if (new_client_fd == ACCEPT_FAILURE) {
    LOG_ERROR("Server::acceptClient_: failed to accept new client connection: "
              << strerror(errno));
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

/**
 * @brief Disconnects a client from the server.
 * 
 * This function disconnects a client from the server by closing its file descriptor,
 * removing it from the clients_ map, removing it from it's channels,
 * and erasing it from the poll_fds vector.
 * 
 * @param poll_fds The vector of pollfd structures representing the active file descriptors.
 * @param it An iterator pointing to the pollfd structure of the client to be disconnected.
 * @return int Returns SUCCESS if the client was successfully disconnected.
 * Otherwise, returns FAILURE.
 */
int Server::disconnectClient_(std::vector<pollfd>& poll_fds,
                              std::vector<pollfd>::iterator& it) {
  // Close client file descriptor
  int client_fd = it->fd;
  Client client = clients_.at(client_fd);
  LOG_DEBUG("Server::disconnectClient_: disconnecting client on fd "
            << client_fd);
  close(client_fd);

  // Remove client from all channels
  std::vector<std::string> channelNames = client.getMyChannels();
  for (std::string channelName : channelNames) {
    Channel channel = channels_.at(channelName);
    channel.partMember(client);
  }
  unsigned long clients_erased = clients_.erase(client_fd);
  if (clients_erased == 0) {
    LOG_WARNING("Server::disconnectClient_: client not found at fd "
                << client_fd << " to erase from clients_ map");
    return FAILURE;
  }

  // Erase client from poll_fds (the poll loop vector)
  poll_fds.erase(it);
  LOG_INFO("Clients remaining on server: " << clients_.size());
  return SUCCESS;
}

/**
 * Sends data from the send buffer to the client.
 * 
 * This function sends the data stored in the send buffer of the specified client
 * to the client's file descriptor using the send system call. It returns the number
 * of bytes sent on success, or an error code on failure.
 * 
 * @param client The client object representing the connected client.
 * @return The number of bytes sent on success, or an error code on failure.
 */
long long Server::sendFromBuffer_(Client& client) {
  std::string& buffer = client.getSendBuffer();
  if (buffer.empty()) {
    return SUCCESS;
  }
  long long send_ret =
      send(client.getFd(), buffer.c_str(), buffer.size(), MSG_NOSIGNAL);
  if (send_ret == SEND_FAILURE) {
    LOG_ERROR("Server::sendFromBuffer_: send failed: " << strerror(errno));
    return SEND_FAILURE;
  }
  LOG_DEBUG("Server::sendFromBuffer_: sent "
            << send_ret << " bytes to client on fd " << client.getFd());
  buffer.erase(0, static_cast<unsigned long>(send_ret));
  return send_ret;
}

/**
 * Receives data from the client and appends it to the receive buffer of the specified client.
 * 
 * @param client The client to receive data from.
 * @return The number of bytes received, or RECV_FAILURE if an error occurred,
 * or RECV_ORDERLY_SHUTDOWN if the client disconnected gracefully.
 */
long long Server::recvToBuffer_(Client& client) {
  char tmpRecvBuffer[SERVER_RECV_BUFFER_SIZE];
  long long recv_ret = RECV_FAILURE;
  std::string& buf = client.getRecvBuffer();

  memset(tmpRecvBuffer, 0, SERVER_RECV_BUFFER_SIZE);
  recv_ret = recv(client.getFd(), tmpRecvBuffer, SERVER_RECV_BUFFER_SIZE, 0);
  if (recv_ret == RECV_FAILURE) {
    LOG_ERROR("Server::recvToBuffer_: recv failed on fd: "
              << client.getFd() << " with: " << strerror(errno));
    return RECV_FAILURE;
  }
  if (recv_ret == RECV_ORDERLY_SHUTDOWN) {
    LOG_INFO("Client on fd " << client.getFd() << " disconnected gracefully");
    client.setWantDisconnect();
    return RECV_ORDERLY_SHUTDOWN;
  }
  LOG_DEBUG("Server::recvToBuffer_: received a packet of "
            << recv_ret << " bytes from client on fd " << client.getFd());
  buf.append(tmpRecvBuffer, static_cast<unsigned long>(recv_ret));
  return recv_ret;
}

/**
 * Extracts a complete message string from the receive buffer of a client.
 * 
 * @param message The extracted message string. (destination)
 * @param client The client from which to extract the message.
 * @return Returns SUCCESS if a complete message is extracted, FAILURE otherwise.
 */
int Server::extractMessageString_(std::string& message, Client& client) {
  std::string& buf = client.getRecvBuffer();
  std::string pattern = "\r\n";
  std::string::size_type pos = buf.find(pattern);
  if (pos == std::string::npos) {
    return FAILURE;
  }
  message.clear();
  message = buf.substr(0, pos);
  buf.erase(0, pos + pattern.length());
  LOG_DEBUG("Server::extractMessageString_: extracted message::"
            << message << ":: from client on fd " << client.getFd());

  return SUCCESS;
}

/**
 * Handles a malformed message received from a client.
 * 
 * @param client The client that sent the malformed message.
 * @param message The malformed message received from the client.
 */
void Server::handleMalformedMessage_(Client& client, Message& message) {
  int numeric = message.getNumeric();
  if (numeric == ERR_CUSTOM_ILLEGALNUL) {
    LOG_WARNING(
        "Server::handleMalformedMessage_: Client on fd "
        << client.getFd()
        << " sent a message that contained a null character: ignoring it");
  } else if (numeric == ERR_INPUTTOOLONG) {
    LOG_WARNING("Server::handleMalformedMessage_: Client on fd "
                << client.getFd()
                << " sent a message that was too long: ignoring it");
  } else if (numeric == ERR_CUSTOM_TOOMANYPARAMS) {
    LOG_WARNING("Server::handleMalformedMessage_: Client on fd "
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
