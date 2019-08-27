#include "echowriter.hpp"

#include <iostream>
#include <cstring>

void EchoWriter::doEchoRequest(std::string message)
{
	std::lock_guard<std::mutex> locker(_mutex);
	_requests.push_back(message);
	_event_happend.notify_one();
}

void EchoWriter::thread_work()
{
	std::unique_lock<std::mutex> locker(_mutex);

	while (_running)
	{
		while (_running && _requests.empty())
			_event_happend.wait(locker);

		while (!_requests.empty())
		{
			auto message = _requests.front();
			_requests.pop_front();

			locker.unlock();
			send_message(message);
			locker.lock();
		}
	}
}

void EchoWriter::send_message(const std::string &message)
{
	/* split and send message */
	auto size = message.size();
	ssize_t pos{0};
	while (pos != size)
	{
		auto bytes_to_send = (size - pos >= 65507) ? 65507 : (size - pos);
		if (_socket->send(message.data() + pos, bytes_to_send))
		{
			std::cerr << "send: " << std::strerror(errno) << std::endl;
			exit(EXIT_FAILURE);
		}

		pos += bytes_to_send;
	}
}
