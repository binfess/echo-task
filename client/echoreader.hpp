#ifndef ECHO_TASK_ECHOREADER_HPP
#define ECHO_TASK_ECHOREADER_HPP

#include <abstractsocket.hpp>

#include "abstractworker.hpp"


class EchoReader : public AbstractWorker
{

	constexpr static int RECV_TIMEOUT_S = 0;
	constexpr static int RECV_TIMEOUT_MS = 500;

public:
	EchoReader(std::shared_ptr<netutils::AbstractSocket> socket) : _socket(socket) { }

private:
	void thread_work() override;

private:
	std::shared_ptr<netutils::AbstractSocket> _socket;

};


#endif //ECHO_TASK_ECHOREADER_HPP
