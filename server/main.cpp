#include <iostream>

#include "echoserver.hpp"

int main(int argc, char **argv)
{
	try
	{
		EchoServer server;
		server.run();
	}
	catch (std::exception &ex)
	{
		std::cerr << ex.what() << std::endl;
		exit(EXIT_FAILURE);
	}

	return 0;
}