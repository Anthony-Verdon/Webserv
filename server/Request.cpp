#include "Request.hpp"
#include <cstdlib>
#include <iostream>

Request::Request(int clientfd)
	: _clientfd(clientfd), _method(-1), _buffer(new char[BUFFER_SIZE])
{}

Request::Request(const Request& other) {
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

static void getQuery(std::string& query, const std::string& name) {
	size_t delimiter = name.find("?");
	if (delimiter != std::string::npos)
		query = name.substr(delimiter + 1);
}

void Request::respondToGetRequest(void) {
	editName(_requestHeader[HEAD]);
	getQuery(_query, _requestHeader[HEAD]);
	std::cout << _query << std::endl;
	DIR* directory = opendir(_requestHeader[HEAD].c_str());
	_isDirectory = false;
	if (directory == NULL) {
		setStatusCode();
		std::ifstream file(_requestHeader[HEAD].c_str(), std::ios::in | std::ios::binary);

		file.seekg(0, std::ios::end);
		std::streampos fileSize = file.tellg();
		file.seekg(0, std::ios::beg);

		std::ostringstream ss;
		ss << "HTTP/1.1 " << _statusCode << "\r\n";
		ss << "Content-type: " << _requestHeader[ACCEPT] << "\r\n";
		ss << "Content-Length: " << fileSize << "\r\n\r\n";

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
		}
		send(_clientfd, ss.str().c_str(), ss.str().size(), 0);
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

static int getMethod(std::string buffer) {
	if (buffer.find("GET") != buffer.npos)	
		return (GET);
	else if (buffer.find("POST") != buffer.npos)	
		return (POST);
	else if (buffer.find("DELETE") != buffer.npos)	
		return (DELETE);
	return (ERROR);
}

void Request::readRequest(std::string const &rawRequest) {
	_method = getMethod(rawRequest);
	parseRequest(rawRequest);
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
}

Request::~Request(void){
	delete [] _buffer;
}
