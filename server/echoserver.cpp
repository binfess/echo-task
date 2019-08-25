#include "echoserver.hpp"

#include <unistd.h>


EchoServer::EchoServer()
{

}

EchoServer::~EchoServer()
{
	if (_epollfd != -1)
	{
		close(_epollfd);
	}
}

int EchoServer::run()
{
	if (_epollfd == -1)
	{
		return -1;
	}

	return event_loop();
}

int EchoServer::event_loop()
{
	return 0;
}
