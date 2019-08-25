#ifndef ECHO_TASK_ECHOSERVER_HPP
#define ECHO_TASK_ECHOSERVER_HPP

#include <iostream>
#include <memory>
#include <unordered_map>
#include <list>
#include <tuple>

#include <sys/epoll.h>

#include <abstractsocket.hpp>
#include <tcpsocket.hpp>
#include <udpsocket.hpp>

class EchoServer
{
	constexpr static ssize_t MAX_EPOLL_EVENTS = 10;

	typedef struct EchoRequest {
		EchoRequest(std::shared_ptr<netutils::TcpSocket> sock1 = nullptr) : sock(sock1) { }
		std::shared_ptr<netutils::TcpSocket> sock;
		std::list<std::pair<size_t, std::string>> requests;
	} echo_client_t;

public:
	EchoServer();
	~EchoServer();

	void run();

private:
	void init_tcp();
	void init_udp();
	void event_loop();

	void accept_connection();
	void process_message(std::string &message);

	void read_tcp_echo(int fd);
	void write_tcp_echo(int fd);

	void read_udp_echo();
	void write_udp_echo();

private:
	int _epollfd = epoll_create1(0);
	int _listenfd{-1};
	int _udpfd{-1};

	std::shared_ptr<netutils::TcpSocket> _tcp_listener = std::make_shared<netutils::TcpSocket>(netutils::NetworkAddress::IPv4);
	std::unordered_map<int, echo_client_t> _tcp_clients;

	std::shared_ptr<netutils::UdpSocket> _udp_socket = std::make_shared<netutils::UdpSocket>(netutils::NetworkAddress::IPv4);
	std::list<std::tuple<size_t, std::string, netutils::NetworkAddress>> _udp_requests;

};


#endif //ECHO_TASK_ECHOSERVER_HPP
