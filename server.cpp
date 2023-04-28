#include "server.hpp"

Server::Server(void)
{
	_socketAddress.sin_family = AF_INET;
	_socketAddress.sin_port = htons(PORT);
	_socketAddress.sin_addr.s_addr = inet_addr(IP_ADRR);
	_socketAddressLen = sizeof(_socketAddress);

	_socketFd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (_socketFd < 0)
		throw ServerException();

	if (bind(_socketFd, (sockaddr*)&_socketAddress, _socketAddressLen) < 0)
		throw ServerException();

	if (listen(_socketFd, LISTEN_BACKLOG) < 0)
		throw ServerException();

	_epollFd = epoll_create(10);
	if (_epollFd < 0)
		throw ServerException();

	std::cout << "Server initialized\n";
}

Server::~Server()
{
	if (close(_epollFd) < 0)
		throw ServerException();
	if (close(_socketFd) < 0)
		throw ServerException();

	std::cout << "Server destroyed\n";
}

void Server::start(void)
{
	std::cout << "starting server\n";

	std::cout << "listening on " << IP_ADRR << ":" << PORT << "\n";
	while (true)
	{
		_acceptConnection();
		_processRequests();
	}
}

std::pair<int, int> Server::_getFile(char const *filePath)
{
	std::ifstream file(filePath, std::ios::binary);
	if (!file.is_open())
		throw ServerException();

	file.seekg(0, file.end);
	std::size_t size = file.tellg();

	int fileFd = open("index.html", O_RDONLY | O_NONBLOCK);
	if (fileFd < 0)
		throw ServerException();

	return std::pair<int, int>(fileFd, size);
}

void Server::_acceptConnection(void)
{
	int clientSocket = accept(_socketFd, (sockaddr*)&_socketAddress, &_socketAddressLen);
	if (clientSocket < 0)
	{
		if (errno != EAGAIN && errno != EWOULDBLOCK)
			throw ServerException();
		return ;
	}

	std::pair<int, int> file = _getFile("index.html");

}
void Server::_processRequests(void)
{
}

const char* Server::ServerException::what(void) const throw()
{
	return strerror(errno);
}
