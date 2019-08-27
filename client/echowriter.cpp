#include "echowriter.hpp"

#include <iostream>

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
	auto size = message.size();
	_socket->send(message.data(), size);
}
