#include "Request.hpp"

void Request::directoryListing(DIR* directory, const std::string& dirName) {
	std::string tmpHTML = "<!DOCTYPE html>\
	<html lang=\"en\">\
	<head>\
		<link rel=\"icon\" href=\"images.png\">\
		<link rel=\"stylesheet\" href=\"style.css\">\
	</head>\
	<body id=\"directory\">\
	<h1>You are on " + dirName + " directory</h1>\
	<p>\
	<ul>\
	";
	(void)dirName;

	struct dirent* file;
	while (true) {	
		file = readdir(directory);
		if (file == NULL)
			break;
		if (file->d_name[0] == '.')
			continue ;
		tmpHTML += "<li><span><a href=\"";
		tmpHTML += file->d_name;
		tmpHTML += "\">";
		tmpHTML += file->d_name;
		tmpHTML += "</a></span></li>\n";
	}
	tmpHTML += "</ul>\
	</p>\
	</body>\
	</html>";
	size_t fileSize = tmpHTML.length();

	std::cout << "Status code: " << _statusCode << std::endl;
	std::ostringstream ss;
	ss << "HTTP/1.1 " << _statusCode << "\r\n";
	if (_requestHeader[LOCATION] != "")
		ss << "Location: " << _requestHeader[LOCATION] << "\r\n";
	ss << "Content-type: " + _requestHeader[ACCEPT] + "\r\n";
	ss << "Content-Length: " << fileSize << "\r\n\r\n";
	ss << tmpHTML;
	write(_clientfd, ss.str().c_str(), ss.str().size());
	ss.str("");
	ss.clear();
	closedir(directory);
}
