#pragma once

#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sstream>
#include <iostream>
#include <cerrno>
#include <csignal>
#include <vector>
#include <exception>
#include <cstring>
#include <string>
#include <sstream>
#include <fcntl.h>
#include <fstream>
#include <utility>
#include <sys/select.h>

#define IP_ADRR "0.0.0.0"
#define PORT 8080
#define LISTEN_BACKLOG 128

class Server {
	public:
		Server(void);
		Server(Server const &other);
		Server &operator=(Server const &other);
		~Server();

		void start(void);
		void emergencyStop(void);

		class ServerException : public std::exception {
			public:
				char const *what(void) const throw();
		};


	private:
		struct _request {
			int			fd;
			bool		isDone;
		};

		int			_socketFd;
		sockaddr_in	_socketAddress;
		socklen_t	_socketAddressLen;
		int			_nbConnections;
		fd_set 		_readSet;

		void _loop(void);
		void _readFile(char const *file, std::string &buffer);
		void _acceptConnection(void);
		void _processRequest(int fd);
};
