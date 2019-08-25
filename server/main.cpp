#include <iostream>

#include "echoserver.hpp"

int main(int argc, char **argv)
{
	EchoServer server;

	if (server.run())
	{
		std::cerr << "Failed to run echo server" << std::endl;
	}

	return 0;
}