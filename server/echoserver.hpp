#ifndef ECHO_TASK_ECHOSERVER_HPP
#define ECHO_TASK_ECHOSERVER_HPP

#include <iostream>
#include <memory>
#include <unordered_map>
#include <list>

#include <sys/epoll.h>

#include <abstractsocket.hpp>
#include <tcpsocket.hpp>

class EchoServer
{
	constexpr static ssize_t MAX_EPOLL_EVENTS = 10;

	typedef struct EchoRequest {
		EchoRequest(std::shared_ptr<netutils::AbstractSocket> sock1 = nullptr) : sock(sock1) { }
		std::shared_ptr<netutils::AbstractSocket> sock;
		std::list<std::pair<size_t, std::string>> requests;
	} echo_client_t;

public:
	EchoServer();
	~EchoServer();

	void run();

private:
	void init_loop();
	void event_loop();

	void accept_connection();
	void read_message(int fd);
	void process_message(std::string &message);
	void write_message(int fd);

private:
	int _epollfd = epoll_create1(0);
	int _listenfd{-1};
	int _udpfd{-1};

	std::shared_ptr<echo_client_t> _udp_socket;
	std::shared_ptr<netutils::TcpSocket> _tcp_listener;
	std::unordered_map<int, echo_client_t> _tcp_clients;

};


#endif //ECHO_TASK_ECHOSERVER_HPP
