#include <iostream>
#include <csignal>

#include "Server.hpp"

void signal_handler(int signum)
{
	(void)signum;
}

//start program and connect to "localhost:8080"
int	main(void)
{
	signal(SIGINT, signal_handler);

	Server server;
	try
	{
		server.start();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
		server.emergencyStop();
	}

	return 0;
}
