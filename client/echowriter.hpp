#ifndef ECHO_TASK_ECHOWRITER_HPP
#define ECHO_TASK_ECHOWRITER_HPP

#include <memory>
#include <list>

#include <abstractsocket.hpp>

#include "abstractworker.hpp"


class EchoWriter : public AbstractWorker
{

public:
	EchoWriter(std::shared_ptr<netutils::AbstractSocket> socket) : _socket(socket) { }

	void doEchoRequest(std::string message);

private:
	void thread_work() final;
	void send_message(const std::string &message);

private:
	std::list<std::string> _requests;
	std::shared_ptr<netutils::AbstractSocket> _socket;

};


#endif //ECHO_TASK_ECHOWRITER_HPP