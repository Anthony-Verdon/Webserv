#pragma once

#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sstream>
#include <iostream>
#include <cerrno>
#include <csignal>
#include <vector>
#include <poll.h>
#include <exception>
#include <cstring>
#include <sstream>
#include <fcntl.h>
#include <fstream>
#include <utility>
#include <sys/epoll.h>

#define IP_ADRR "0.0.0.0"
#define PORT 8080
#define LISTEN_BACKLOG 10

class Server {
	public:
		Server(void);
		~Server();

		void start(void);

		class ServerException : public std::exception {
			public:
				const char* what(void) const throw();
		};

	private:
		int								_socketFd;
		sockaddr_in						_socketAddress;
		socklen_t						_socketAddressLen;
		int								_epollFd;

		std::pair<int, int> Server::_getFile(char const *file);
		void _acceptConnection(void);
		void _processRequests(void);
};
