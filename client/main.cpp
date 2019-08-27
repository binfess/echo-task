#include <iostream>
#include <cstring>
#include <unistd.h>

#include <tcpsocket.hpp>
#include <udpsocket.hpp>

#include "echowriter.hpp"
#include "echoreader.hpp"


int main(int argc, char **argv)
{
	using namespace netutils;

	NetworkAddress server_address(NetworkAddress::IPv4, "127.0.0.1", 45000);
	std::shared_ptr<AbstractSocket> socket;

	bool use_udp{0};
	int opt{0};
	while ((opt = getopt(argc, argv, "u")) != -1)
	{
		switch (opt)
		{
			case 'u':
				use_udp = 1;
				break;
			default: /* '?' */
				std::cerr << "Usage: " << argv[0] << " [-u]" << std::endl;
				exit(EXIT_FAILURE);
		}
	}

	if (use_udp)
	{
		socket = std::make_shared<UdpSocket>(NetworkAddress::IPv4);
	}
	else
	{
		socket = std::make_shared<TcpSocket>(NetworkAddress::IPv4);
	}

	std::cout << "Used connection: " << ((use_udp) ? "UDP" : "TCP") << std::endl;

	if (socket->connect(server_address))
	{
		std::cerr << "connect: " << std::strerror(errno) << std::endl;
		exit(EXIT_FAILURE);
	}

	{
		EchoWriter writer(socket);
		EchoReader reader(socket);

		reader.start();
		writer.start();

		std::string message;
		while (!std::getline(std::cin, message).eof())
		{
			writer.doEchoRequest(message);
		}
	}

	return 0;
}