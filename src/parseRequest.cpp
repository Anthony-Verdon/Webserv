#include <string>

#include "Request.hpp"

void trimString(std::string& string, const char* charset);
static std::string getToken(const std::string& str, char sep, int pos){
	std::string token;
	int cur_pos;

	cur_pos = 1;
	for (uint64_t i = 0; i < str.size(); i++) {
		if (str[i] == sep)	
		{
			if (cur_pos == pos)
				break;
			token.clear();
			cur_pos++;
		}
		else
			token += str[i];
	}
	return (token);
}

static std::string getExtension(const std::string& fileName) {
	std::string extension;
	size_t dot = fileName.find(".");
	if (dot != std::string::npos) {
		extension = fileName.substr(dot + 1, fileName.find(",", dot));
	}
	else {
		extension = "text/html";
	}
	return (extension);
}

void Request::processLine(std::string line, int lineToken) {
	std::string str = getToken(line, ' ', 2);
	int pos = str.find('\r');
	std::string root = "www";
	if (pos > 0)
		str.erase(pos, 1);
	switch (lineToken) {
		case HEAD:
			{
				str.erase(0, 1);
#if DEBUG
				std::cout << str << "\n";
#endif
				if (line.find("HTTP/1.1") != std::string::npos)
					_validRequest = true;
				else
					_validRequest = false;
				if (_method == "GET" && str.find(root) == std::string::npos) {
					if (str.length() == 0)
						_requestHeader.insert(strPair(HEAD, root + "/index.html"));
					else
						_requestHeader.insert(strPair(HEAD, root + "/" + str));
				}
				else if (_method == "GET" && str.find(root) != std::string::npos) {
					_requestHeader.insert(strPair(HEAD, str));
				}
				else if (_method == "POST" && str == "/")
					_requestHeader.insert(strPair(HEAD, "www/uploads/default"));
				else if (_method == "POST") {
					str.erase(0, 1);
					_requestHeader.insert(strPair(HEAD, str));
				}
			}
			break;
		case BOUNDARY:
			{
				std::string tmp = getToken(line, ' ', 3);
				_boundary = tmp.substr(tmp.find("=") + 1, tmp.length());
				trimString(_boundary, "-\r");
			}
			break;
		case ACCEPT:
			str = getExtension(_requestHeader[HEAD]);
			break;
		default:
			break;
	}
	if (lineToken != HEAD && lineToken != BOUNDARY)
		_requestHeader.insert(strPair(lineToken, str));
}

static int getLineToken(std::string line) {
	if (line.find("POST") != std::string::npos || line.find("GET") != std::string::npos || line.find("DELETE") != std::string::npos)
		return (HEAD);
	else if (line.find("Host:") != std::string::npos)
		return (HOST);
	else if (line.find("User-Agent:") != std::string::npos)
		return (USER_AGENT);
	else if (line.find("Accept:") != std::string::npos)
		return (ACCEPT);
	else if (line.find("Accept-Language:") != std::string::npos)
		return (ACCEPT_LANGUAGE);
	else if (line.find("Accept-Encoding:") != std::string::npos)
		return (ACCEPT_ENCODING);
	else if (line.find("Content-Type:") != std::string::npos) {
		if (line.find("boundary") != std::string::npos)
			return (BOUNDARY);
		return (CONTENT_TYPE);
	}
	else if (line.find("Content-Length:") != std::string::npos)
		return (CONTENT_LENGTH);
	else if (line.find("Transfer-Encoding:") != std::string::npos)
		return (TRANSFER_ENCODING);
	else
		return (-2);
}

static void processBody(std::string& boundary, std::string& line, strMap& requestHeader) {
	int i = 1;
	std::string str = getToken(line, '\n', i);
	trimString(str, "-\r");
	if (str == boundary) {
		i++;
		while (i < 4) {
			str = getToken(line, '\n', i);
			if (i == 2) {
				std::string tmp = getToken(str, '\"', 4);
				requestHeader[HEAD] = tmp;
			}
			else if (i == 3) {
				std::string tmp = getToken(str, ' ', 2);
				requestHeader[CONTENT_TYPE] = tmp;
			}
			i++;
		}
		std::string tmp = line.substr(line.find("\r\n\r\n"), line.length());
		tmp.erase(0, 4);
		tmp.erase(tmp.end() - 1);
		size_t pos = tmp.rfind("\n------------------");
		tmp.erase(pos, pos - tmp.length());
		tmp.erase(tmp.end() - 1);
#if DEBUG
		std::cout << tmp;
#endif
		requestHeader[BODY].clear();
		requestHeader[BODY] = tmp;
	}
	else {
		std::cout << str << std::endl;
	}
}

void Request::parseHeader(const std::string& buffer) {
	size_t i = 0;
	int lineToken;
	std::string line;

	while (buffer[i])
	{
		while (buffer[i] && buffer[i] != '\n')
		{
			line += buffer[i];
			i++;
		}
		if (line == "\r")
			break ;
		lineToken = getLineToken(line);
		processLine(line, lineToken);
		line.clear();
		i++;
	}
	line.clear();
	size_t pos = buffer.find("\r\n\r\n");
	if (pos == std::string::npos)
		throw std::invalid_argument("invalid header");
	line = buffer.substr(pos + 4, std::string::npos);
	_requestHeader[BODY] = line;
}

bool Request::parseChunkedBody(const std::string& buffer) {
	static bool bufferFull = false;
	static std::string tmpBody = "";
	if (!bufferFull)
	{
		tmpBody += buffer;
		if (buffer.find("\r\n0\r\n", 0) != std::string::npos)
			bufferFull = true;
		else
			return (false);
	}

	std::size_t pos = 0;
	while (true) {
		std::size_t limPos = tmpBody.find("\r\n", pos);
		std::string tmp = tmpBody.substr(pos, limPos - pos);
		long size = std::strtol(tmp.c_str(), NULL, 16);
		if (size == 0) {
			std::cout << "Body finished" << std::endl;
			tmpBody.clear();
			bufferFull = false;
			return (true);
		}
		_requestHeader[BODY] += tmpBody.substr(limPos + 2, size);
		pos = limPos + 2 + size + 2;
	}
}

bool Request::parseBody(const std::string& buffer) {
	if (_requestHeader[TRANSFER_ENCODING] == "chunked")
		return (parseChunkedBody(buffer));

	_requestHeader[BODY] += buffer;
	size_t pos = buffer.find(_boundary);
	if (pos != std::string::npos && pos > _boundary.length() + 30) {
#if DEBUG
		std::cout << "Body finished" << std::endl;
#endif
		return (true);
	}
	return (false);
}

void Request::respondToRequest(void) {
	if (_method == "GET")
		respondToGetRequest();	
	else if ("POST") {
		processBody(_boundary, _requestHeader[BODY], _requestHeader);
		respondToPostRequest();	
	}
	else if ("DELETE")
		respondToDeleteRequest();	
	_requestHeader.clear();
}
