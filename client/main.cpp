#include <iostream>

#include <tcpsocket.hpp>
#include <udpsocket.hpp>

#include "echowriter.hpp"

#include <chrono>

int main()
{
	using namespace netutils;

	NetworkAddress server_address(NetworkAddress::IPv4, "127.0.0.1", 45000);

	std::shared_ptr<AbstractSocket> socket = std::make_shared<UdpSocket>(NetworkAddress::IPv4);
	socket->connect(server_address);

	{
		EchoWriter writer(socket);

		writer.start();

		writer.doEchoRequest("test");
		writer.doEchoRequest("123123");

		std::this_thread::sleep_for(std::chrono::seconds(3));
	}

	return 0;
}