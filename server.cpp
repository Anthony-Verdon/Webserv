#include "server.hpp"

Server::Server(void) {
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

	_epollFd = epoll_create(WORKER_NB);
	if (_epollFd < 0)
		throw ServerException();
}

Server::Server(Server const &other) {
	*this = other;
}

Server &Server::operator=(Server const &other) {
	_socketFd = other._socketFd;
	_socketAddress = other._socketAddress;
	_socketAddressLen = other._socketAddressLen;
	_epollFd = other._epollFd;
	return *this;
}

Server::~Server() {
	close(_epollFd);
	close(_socketFd);
}

void Server::start(void) {
	std::cout << "listening on " << IP_ADRR << ":" << PORT << "\n";
	while (true)
	{
		_acceptConnection();
		_processRequests();
	}
}

void Server::_readFile(char const *filePath, std::string &buffer) {
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

void Server::_acceptConnection(void) {
	int clientSocket = accept(_socketFd, (sockaddr*)&_socketAddress, &_socketAddressLen);
	if (clientSocket < 0)
	{
		if (errno != EAGAIN && errno != EWOULDBLOCK)
			throw ServerException();
		return ;
	}

	epoll_event event;
	event.events = EPOLLOUT;

	struct _request *req = new struct _request;
	req->fd = clientSocket;
	req->isDone = false;
	event.data.ptr = req;

	if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, clientSocket, &event) < 0)
		throw ServerException();

	std::cout << "accepted connection\n";
}

void Server::_processRequests(void) {
	epoll_event events[WORKER_NB];
	int n = epoll_wait(_epollFd, events, WORKER_NB, 10);
	if (n < 0)
		throw ServerException();
	if (n == 0)
		return ;

	for (int i = 0; i < n; i++)
	{
		if (events[i].events & (EPOLLERR | EPOLLHUP))
			throw ServerException();

		if (events[i].events & EPOLLOUT)
		{
			struct _request *req = static_cast<struct _request *>(events[i].data.ptr);
			if (!req->isDone)
			{
				std::cout << "processing request\n";

				_readFile("index.html", req->buffer);

				std::ostringstream oss;
				oss << "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: "
					<< req->buffer.size() << "\r\n\r\n" << req->buffer;
				if (write(req->fd, oss.str().c_str(), oss.str().size()) < 0)
					throw ServerException();

				req->isDone = true;
				std::cout << "sent response\n";
			}
			else
			{
				if (epoll_ctl(_epollFd, EPOLL_CTL_DEL, req->fd, NULL) < 0)
					throw ServerException();
				close(req->fd);
				delete req;
				std::cout << "closing connection\n";
			}
		}
	}
}

const char* Server::ServerException::what(void) const throw() {
	return strerror(errno);
}
