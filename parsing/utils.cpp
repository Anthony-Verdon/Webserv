#include <sstream>
#include <vector>
#include <dirent.h>

#define WHITESPACE " \n\r\t\f\v"

const std::string ParsingError(std::string error) throw()
{
	return ("parsing error: " + error + "\n");
}

const std::string intToString(const int number)
{
	std::ostringstream stringStream;
	stringStream << number;
	return (stringStream.str());
}

const std::vector<std::string> splitString(const std::string string, const char delimiter)
{
	std::stringstream 			ss(string);
	std::vector<std::string>	strings;
	std::string 				buffer;
	
	while(getline(ss, buffer, delimiter))
    	strings.push_back(buffer);
	return (strings);
}

void	trimString(std::string & string)
{
	size_t start = string.find_first_not_of(WHITESPACE);
	size_t end = string.find_last_not_of(WHITESPACE);
	if (start != std::string::npos)
		string = string.substr(start, end - start + 1);
	else
		string = "";
}

bool isDigit(const std::string string)
{
	for (int i = 0; string[i]; i++)
	{
		if (string[i] < '0' || string[i] > '9')
			return (false);
	}
	return (true);
}

bool isValidPath(const std::string string)
{
	DIR *directory = opendir(string.c_str());
	if (directory != NULL)
	{
		closedir(directory);
		return (true);
	}
	return (false);
}
