#include "echoreader.hpp"

#include <iostream>

#include <sys/epoll.h>

void EchoReader::thread_work()
{
	std::unique_lock<std::mutex> locker(_mutex);

	timeval tv = {
			.tv_sec = RECV_TIMEOUT_S,
			.tv_usec = RECV_TIMEOUT_MS * 1000
	};
	if (setsockopt(_socket->handle(), SOL_SOCKET, SO_RCVTIMEO, (const char*) &tv, sizeof tv))
	{
		std::cout << "EchoReader:: Failed to settings socket" << std::endl;
	}

	std::array<char, 65507> buffer;
	while (_running)
	{
		locker.unlock();

		auto size = buffer.size();
		if (_socket->recv(buffer.data(), size))
		{
			locker.lock();
			continue;
		}

		std::string message(buffer.data(), size);
		std::cout << message << std::endl;

		locker.lock();
	}
}
