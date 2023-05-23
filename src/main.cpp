#include <iostream>
#include <csignal>
#include <cstdlib>
#include <vector>

#include "Server.hpp"
#include "parsing/parsing.hpp"

char **g_env;
void signal_handler(int signum)
{
	(void)signum;
	throw std::runtime_error("SIGINT received");
}

int	main(int argc, char **argv, char **envp)
{
	signal(SIGINT, signal_handler);
	
	g_env =  envp;
	std::vector<t_server> serverConfigFile;
	try
	{
		if (argc == 2)
			serverConfigFile = Parsing::parseConfFile(argv[1]);
		else
			serverConfigFile = Parsing::parseConfFile("conf/easy.conf");
	}
	catch(const std::string exception)
	{
		std::cerr << exception;
		return EXIT_FAILURE;
	}
	Server server;
	try
	{
		for (std::vector<t_server>::iterator it = serverConfigFile.begin(); it != serverConfigFile.end(); ++it)
			server.addAddress(*it);
		server.start();
	}
	catch(const Server::ServerException& e)
	{
		std::cerr << e.what() << '\n';
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
