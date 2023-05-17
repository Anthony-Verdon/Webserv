#pragma once

#include <string>
#include <map>
#include <dirent.h>
#include <sys/socket.h>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <iostream>
#include <arpa/inet.h>

typedef std::map<int, std::string> strMap;
typedef std::pair<int, std::string> strPair;

enum methods {
	ERROR,
	GET,
	POST,
	DELETE
};

enum attributes {
	BODY,
	HEAD,
	HOST,
	USER_AGENT,
	ACCEPT,
	LOCATION,
	ACCEPT_LANGUAGE,
	ACCEPT_ENCODING,
	CONTENT_TYPE,
	BOUNDARY,
	CONTENT_LENGTH,
	ORIGIN,
	CONNECTION,
	REFERER,
	UPGRADE_INSECURE_REQUESTS,
	SEC_FETCH_DEST,
	SEC_FETCH_MODE,
	SEC_FETCH_SITE,
	SEC_FETCH_USER,
	PRAGMA,
	CACHE_CONTROL
};

#define BUFFER_SIZE 1024

class Request {
	public:
		Request(int clientfd);
		~Request(void);

		void readRequest(std::string const &rawRequest);


	private:
		Request(void);
		Request(const Request& other);
		Request& operator=(const Request& other);
		void respondToGetRequest(void);
		void respondToPostRequest(void);
		void respondToDeleteRequest(void);
		void errorOnRequest(void);
		void processLine(std::string line, int lineToken);
		void parseRequest(std::string request);
		bool setStatusCode(void);
		void directoryListing(DIR* directory, const std::string& dirName);

	private:
		int	_clientfd;
		int _method;
		std::string _statusCode;
		std::string _boundary;
		std::string _query;
		const char* _buffer;
		strMap _requestHeader;
		bool _isDirectory;
};
