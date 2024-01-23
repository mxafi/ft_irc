# ft_irc - Internet Relay Chat Server

## Description:

ft_irc is an IRC (Internet Relay Chat) server implemented in C++. The server is designed to handle multiple clients simultaneously, supporting basic IRC functionalities such as authentication, nickname and username setting, channel joining, private messaging, and more.

This project is a part of the 42 curriculum at Hive Helsinki.

## Collaborators

- [Lionel Clerc](https://github.com/liocle)
- [Diego James](https://github.com/djames9)
- [Markus Laaksonen](https://github.com/mxafi)

## Usage:

```bash
./ircserv <port> <password>
```

- port: The port number on which the IRC server will listen for incoming connections.
- password: Connection password required by IRC clients to connect to the server.

## Compiling and running:

1. Clone the repository.
2. Compile the server using the provided Makefile.
3. Run the executable.

```bash
git clone https://github.com/mxafi/ft_irc
cd ft_irc && make
./ircserv <port> <password>
```

## Contributing:

Contributions to this project are welcome. Feel free to submit issues or pull requests.

## License:

This project is licensed under the MIT License.
