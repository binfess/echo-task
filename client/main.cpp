#include <iostream>
#include <chrono>

#include <tcpsocket.hpp>
#include <udpsocket.hpp>

#include "echowriter.hpp"
#include "echoreader.hpp"


int main()
{
	using namespace netutils;

	NetworkAddress server_address(NetworkAddress::IPv4, "127.0.0.1", 45000);

	std::shared_ptr<AbstractSocket> socket = std::make_shared<UdpSocket>(NetworkAddress::IPv4);
	socket->connect(server_address);

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