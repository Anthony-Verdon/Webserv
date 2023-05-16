#include "server.hpp"

Server::Server(void)
	: _nbConnections(0)
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

	FD_ZERO(&_readSet);
	FD_SET(_socketFd, &_readSet);
}

Server::Server(Server const &other)
	: _socketFd(other._socketFd), _socketAddress(other._socketAddress), _socketAddressLen(other._socketAddressLen)
{}

Server &Server::operator=(Server const &other)
{
	_socketFd = other._socketFd;
	_socketAddress = other._socketAddress;
	_socketAddressLen = other._socketAddressLen;
	return *this;
}

Server::~Server()
{
	close(_socketFd);
}

void Server::start(void)
{
	fd_set readSet;

	std::cout << "listening on " << IP_ADRR << ":" << PORT << "\n";
	while (true)
	{
		readSet = _readSet;

		if (select(WORKER_NB + 1, &readSet, NULL, NULL, NULL) < 0)
			throw ServerException();

		for (int i = 0; i < WORKER_NB; i++)
		{
			if (FD_ISSET(i, &readSet))
			{
				if (i == _socketFd)
					_acceptConnection();
				else
					_processRequest(i);
			}
		}
	}
}

void Server::_readFile(char const *filePath, std::string &buffer)
{
	std::ifstream file(filePath, std::ios::binary);
	if (!file.is_open())
		throw ServerException();

	file.seekg(0, file.end);
	std::size_t size = file.tellg();
	file.seekg(0, file.beg);

	buffer.resize(size);
	file.read(&buffer[0], size);

	file.close();
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

	FD_SET(clientSocket, &_readSet);
	++_nbConnections;
	std::cout << "accepted connection\n";
}

void Server::_processRequest(int fd)
{
	char header_buffer[1024];
	int rc = recv(fd, header_buffer, 1024, 0);
	if (rc == -1)
		throw ServerException();
	else if (rc == 0)
	{
		close(fd);
		FD_CLR(fd, &_readSet);
		--_nbConnections;
		std::cout << "closed connection\n";
		return ;
	}
	std::cout << rc << " bytes read\n";

	std::string response_buffer;
	_readFile("index.html", response_buffer);

	std::ostringstream oss;
	oss << "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: "
		<< response_buffer.size() << "\r\n\r\n" << response_buffer;
	if (write(fd, oss.str().c_str(), oss.str().size()) < 0)
		throw ServerException();
	std::cout << "sent response\n";
}

char const *Server::ServerException::what(void) const throw() {
	return strerror(errno);
}
