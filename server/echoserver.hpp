#ifndef ECHO_TASK_ECHOSERVER_HPP
#define ECHO_TASK_ECHOSERVER_HPP

#include <sys/epoll.h>

class EchoServer
{

public:
	EchoServer();
	~EchoServer();

	int run();

private:
	int event_loop();

private:
	int _epollfd = epoll_create1(0);

};


#endif //ECHO_TASK_ECHOSERVER_HPP
