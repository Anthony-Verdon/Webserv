#include "Request.hpp"
#include <cstdlib>
#include <iostream>

Request::Request(const Server& other): Server(other), _buffer(new char[BUFFER_SIZE]) {
}

Request::Request(const Request& other): Server("0.0.0.0", 8080) {
	(void)other;
}

Request&	Request::operator=(const Request& other) {
	(void)other;
	return (*this);
}

static void editName(std::string& name) {
	while (1) {
		char* name_c = (char *)name.c_str();
		size_t len;
		if (name.find("%") != std::string::npos) {
			for (size_t i = 0; name_c[i]; i++) {
				if (name_c[i] == '%') {
					name_c = &(name_c[i + 1]);
					break ;
				}
			}
			long c = std::strtol(name_c, NULL, 16);
			for (len = 0; std::isdigit(name_c[len]); len++);
			name.replace(name.find("%"), len + 1, (const char*)&c);
		}
		else if (name.find("+") != std::string::npos) {
			name.replace(name.find("+"), 1, " ");
		}
		else
			break ;
	}
}

void Request::respondToGetRequest(void) {
	editName(_requestHeader[HEAD]);
	std::cout << "Name: " << _requestHeader[HEAD] << std::endl;
	DIR* directory = opendir(_requestHeader[HEAD].c_str());
	_isDirectory = false;
	if (directory == NULL) {
		setStatusCode();
		std::ifstream file(_requestHeader[HEAD].c_str(), std::ios::in | std::ios::binary);

		file.seekg(0, std::ios::end);
		long fileSize = file.tellg();
		file.seekg(0, std::ios::beg);

		std::ostringstream ss;
		ss << "HTTP/1.1 " << _statusCode << "\r\n";
		ss << "Content-type: " << _requestHeader[ACCEPT] << "\r\n";
		ss << "Content-Length: " << fileSize << "\r\n\r\n";
		write(_clientfd, ss.str().c_str(), ss.str().size());
		ss.str("");
		ss.clear();

		while (!file.eof()) {
			if (fileSize > BUFFER_SIZE) {
				file.read((char *)_buffer, BUFFER_SIZE);
				ss.write(_buffer, BUFFER_SIZE);
				fileSize -= BUFFER_SIZE;
			}
			else {
				file.read((char *)_buffer, fileSize);
				ss.write(_buffer, fileSize);
			}
			write(_clientfd, ss.str().c_str(), ss.str().size());
			ss.str("");
			ss.clear();
		}
		file.close();
	}
	else {
		_isDirectory = true;
		setStatusCode();
		directoryListing(directory, _requestHeader[HEAD]);
	}
}

void Request::respondToPostRequest(void) {
	setStatusCode();
	std::string body = _requestHeader[BODY];
	std::fstream file(_requestHeader[HEAD].c_str(), std::fstream::out);

	std::ostringstream ss;
	ss << "HTTP/1.1 " << _statusCode << "\r\n";
	ss << "Content-type: " + _requestHeader[ACCEPT] + "\r\n";
	ss << "Content-Length: " << _requestHeader[CONTENT_LENGTH] << "\r\n\r\n";
	write(_clientfd, ss.str().c_str(), ss.str().size());
	file << body;
	file.close();
}

void Request::respondToDeleteRequest(void) {
	setStatusCode();
	std::string body = _requestHeader[BODY];
	std::fstream file(_requestHeader[HEAD].c_str(), std::fstream::out);

	std::ostringstream ss;
	ss << "HTTP/1.1 " << _statusCode << "\r\n";
	ss << "Content-type: " + _requestHeader[ACCEPT] + "\r\n";
	ss << "Content-Length: " << _requestHeader[CONTENT_LENGTH] << "\r\n\r\n";
	write(_clientfd, ss.str().c_str(), ss.str().size());
	file << body;
	file.close();
}

void Request::acceptRequest(void) {
	while (true)
	{
		//when someone connect, it accept it
		_clientfd = accept(_sockfd, (sockaddr*)&_sockAddr, &_sockAddr_len);
		if (_clientfd < 0) {
			std::ostringstream ss;
			ss <<
			"Request failed to accept incoming connection from ADDRESS: "
			<< inet_ntoa(_sockAddr.sin_addr) << "; PORT: "
			<< ntohs(_sockAddr.sin_port);
			exitWithError("error: log\n");
		}
		readRequest();
		close(_clientfd);
	}
}

static int getMethod(std::string buffer) {
	if (buffer.find("GET") != buffer.npos)	
		return (GET);
	else if (buffer.find("POST") != buffer.npos)	
		return (POST);
	else if (buffer.find("DELETE") != buffer.npos)	
		return (DELETE);
	return (ERROR);
}

void Request::readRequest(void) {
	char* buffer_c = new char[BUFFER_SIZE];
	std::string buffer;
	int	end;
	if ((end = read(_clientfd, buffer_c, (BUFFER_SIZE) - 1)) < 0)
		exitWithError("error: failed to read client socket\n");
	write(1, buffer_c, end);
	buffer.assign(buffer_c, end);
	_method = getMethod(buffer);
	parseRequest(buffer);
	switch (_method) {
		case GET:
			respondToGetRequest();	
			break;
		case POST:
			respondToPostRequest();	
			break;
		case DELETE:
			respondToDeleteRequest();	
			break;
		default:
			break;
	}
	_requestHeader.clear();
	delete []buffer_c;
}

Request::~Request(void){
	close(_clientfd);
	delete [] _buffer;
}
