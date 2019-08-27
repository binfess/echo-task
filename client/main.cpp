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

		writer.doEchoRequest("test");
		writer.doEchoRequest("123123");

		reader.stop();
		std::cout << "reader stop" << std::endl;

		writer.stop();
		std::cout << "writer stop" << std::endl;
	}

	return 0;
}