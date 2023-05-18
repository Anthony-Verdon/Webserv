#include <cstdlib>
#include <algorithm>

#include "Server.hpp"

Server::Server(void)
	: _addressLen(sizeof(sockaddr_in))
{
	FD_ZERO(&_readSet);
}

void Server::addAddress(std::string const &address, int port)
{
	sockaddr_in socketAddress;
	socketAddress.sin_family = AF_INET;
	socketAddress.sin_port = htons(port);
	socketAddress.sin_addr.s_addr = inet_addr(address.c_str());

	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0)
		throw ServerException();

	int option = 1;
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(int)) < 0)
		throw ServerException();

	if (bind(fd, (sockaddr*)&socketAddress, _addressLen) < 0)
		throw ServerException();

	if (listen(fd, LISTEN_BACKLOG) < 0)
		throw ServerException();

	FD_SET(fd, &_readSet);
	_sockets[fd] = socketAddress;
}

Server::Server(Server const &other)
{
	(void)other;
}

Server &Server::operator=(Server const &other)
{
	(void)other;
	return *this;
}

Server::~Server()
{
	for (socketMap::iterator it = _sockets.begin(); it != _sockets.end(); ++it)
		close(it->first);
	for (requestMap::iterator it = _requests.begin(); it != _requests.end(); ++it)
		close(it->first);
}

void Server::start(void)
{
	fd_set readSet;

	for (socketMap::iterator it = _sockets.begin(); it != _sockets.end(); ++it)
		std::cout << "Listening on " << inet_ntoa(it->second.sin_addr) << ":" << ntohs(it->second.sin_port) << std::endl;
	while (true)
	{
		readSet = _readSet;
		if (select(FD_SETSIZE + 1, &readSet, NULL, NULL, NULL) < 0)
			throw ServerException();

		for (int i = 0; i < FD_SETSIZE; i++)
		{
			if (FD_ISSET(i, &readSet))
			{
				socketMap::iterator socket = _sockets.find(i);
				if (socket != _sockets.end())
				{
					_acceptConnection(socket->first, &socket->second);
					continue ;
				}

				requestMap::iterator request = _requests.find(i);
				if (request != _requests.end())
				{
					_processRequest(request->first, request->second);
					continue ;
				}

				throw std::runtime_error("Unknown file descriptor");
			}
		}
	}
}

void Server::_acceptConnection(int socketFd, sockaddr_in *address)
{
	int fd = accept(socketFd, (sockaddr *)address, &_addressLen);
	if (fd < 0)
	{
		if (errno != EAGAIN && errno != EWOULDBLOCK)
			throw ServerException();
		return ;
	}

	FD_SET(fd, &_readSet);
	_requests[fd] = Request(fd);
	std::cout << "accepted connection\n";
}

void Server::_processRequest(int clientFd, Request &request)
{
	std::string header_buffer(1024, 0);
	int rc = recv(clientFd, &header_buffer[0], 1024, 0);
	if (rc <= 0)
	{
		close(clientFd);
		FD_CLR(clientFd, &_readSet);
		_requests.erase(clientFd);
		if (rc < 0)
			std::cerr << "error: " << strerror(errno) << "\n";
		std::cout << "closed connection\n";
		return ;
	}
	std::cout << rc << " bytes read\n";

	if (request.readRequest(header_buffer))
	{
		
	}
	std::cout << "sent response\n";
}

char const *Server::ServerException::what(void) const throw() {
	return strerror(errno);
}
