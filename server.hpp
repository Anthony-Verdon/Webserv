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
#include <string>
#include <sstream>
#include <fcntl.h>
#include <fstream>
#include <utility>
#include <sys/epoll.h>

#define IP_ADRR "0.0.0.0"
#define PORT 8080
#define LISTEN_BACKLOG 10
#define WORKER_NB 1000

class Server {
	public:
		Server(void);
		Server(Server const &other);
		Server &operator=(Server const &other);
		~Server();

		void start(void);

		class ServerException : public std::exception {
			public:
				const char* what(void) const throw();
		};


	private:
		struct _request {
			int			fd;
			std::string buffer;
			bool		isDone;
		};

		int			_socketFd;
		sockaddr_in	_socketAddress;
		socklen_t	_socketAddressLen;
		int			_epollFd;

		void _readFile(char const *file, std::string &buffer);
		void _acceptConnection(void);
		void _processRequests(void);
};
