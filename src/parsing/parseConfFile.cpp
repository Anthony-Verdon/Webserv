#include "parsing.hpp"
#include <fstream>
#include <cstring>

void	Parsing::checkDifferentServer(std::vector<t_server> servers)
{
	for (size_t i = 0; i < servers.size(); i++)
	{
		for (size_t j = i + 1; j < servers.size(); j++)
		{
			if (servers[i].port == servers[j].port && servers[i].host == servers[j].host)
				throw (ParsingError("multiple server with same address."));
		}
	}
}
const std::vector<t_server> Parsing::readConfFile(std::ifstream & confFile)
{
	std::vector<t_server> servers;

	int 						nbLine = 0;
	bool 						inServerBlock = false;
	bool 						inLocationBlock = false;
	bool 						inMethodBlock = false;
	std::string 				line;
	t_server					server;
	t_location					location;
	while (std::getline(confFile, line))
	{
		nbLine++;
		trimString(line, WHITESPACE);
		line = line.substr(0, line.find("##"));
		if (line.length() == 0)
			continue;
		else if (!inServerBlock && !inLocationBlock && !inMethodBlock)
		{
			if (line.length() > 0 && line != "server {")
				throw(ParsingError("line " + intToString(nbLine) + " is invalid."));
			inServerBlock = true;
			initializeServer(server);
		}
		else if (inServerBlock && !inLocationBlock && !inMethodBlock)
		{
			if (strncmp(line.c_str(), "location", 8) == 0)
			{
				std::vector<std::string> lineSplitted = splitString(line, ' ');
				if (lineSplitted.size() != 3)
					throw(ParsingError("line " + intToString(nbLine) + " is invalid."));
				location.locationPath = lineSplitted[1];
				initializeLocation(location);
				inLocationBlock = true;
			}
			else if (line == "}")
			{
				servers.push_back(server);
				inServerBlock = false;
			}
			else
				parseLineServerBlock(line, nbLine, server);
		}
		else if (inServerBlock && inLocationBlock && !inMethodBlock)
		{
			if (strncmp(line.c_str(), "methods", 7) == 0)
				inMethodBlock = true;
			else if (line == "}")
			{
				testLocationValue(location);
				server.locations.push_back(location);
				inLocationBlock = false;
			}
			else
				parseLineLocationBlock(line, nbLine, location);
		}
		else if (inServerBlock && inLocationBlock && inMethodBlock)
		{
			if (line == "}")
				inMethodBlock = false;
			else
				parseLineMethodBlock(line, nbLine, location);
		}
	}
	confFile.close();
	checkDifferentServer(servers);
	return (servers);
}

const std::vector<t_server> Parsing::parseConfFile(const std::string path)
{
	if (path.find(".conf") + 5 != path.length())
		throw (ParsingError("configuration file must have \".conf\" extension."));
	std::ifstream confFile(path.c_str(), std::ios_base::in);
	if (!confFile.is_open())
		throw (ParsingError("can't open configuration file."));
	return (readConfFile(confFile));
}
