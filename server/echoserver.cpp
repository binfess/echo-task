#include "echoserver.hpp"

#include <unistd.h>
#include <cstring>

#include <tcpsocket.hpp>


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
	if (_epollfd == -1 || init_loop())
	{
		return -1;
	}

	return event_loop();
}

int EchoServer::init_loop()
{
	auto listener = std::make_shared<netutils::TcpSocket>(netutils::NetworkAddress::IPv4);
	listener->setSocketOption(netutils::TcpSocket::ReuseAddressOption, 1);
	listener->setNonBlocking(true);

	if (listener->bind(netutils::NetworkAddress(netutils::NetworkAddress::IPv4, "0.0.0.0", 45000)))
	{
		std::cerr << strerror(errno) << std::endl;
		return -1;
	}

	if (listener->listen(10))
	{
		std::cerr << strerror(errno) << std::endl;
		return -1;
	}

	_listenfd = listener->handle();
	epoll_event ev = {
			.events = EPOLLIN,
			.data = { .fd = _listenfd }
	};

	if (epoll_ctl(_epollfd, EPOLL_CTL_ADD, _listenfd, &ev))
	{
		std::cerr << strerror(errno) << std::endl;
		return -1;
	}

	_handles.emplace(_listenfd, echo_client_t(listener));

	return 0;
}


int EchoServer::event_loop()
{
	epoll_event events[MAX_EPOLL_EVENTS];
	std::array<char, 65507> messageBuffer{0};

	for (;;)
	{
		int nfds = epoll_wait(_epollfd, events, MAX_EPOLL_EVENTS, -1);
		if (nfds == -1)
		{
			break;
		}

		for (int n = 0; n < nfds; ++n)
		{
			if (events[n].data.fd == _listenfd)
			{
				echo_client_t &client = _handles[_listenfd];
				accept_connection(std::dynamic_pointer_cast<netutils::TcpSocket>(client.sock));
			}
			else if (events[n].events & EPOLLIN)
			{
				std::cout << "ready to read" << std::endl;
			}
			else
			{
				std::cout << "ready to cho-nibudb" << std::endl;
			}
		}
	}

	return 0;
}

int EchoServer::accept_connection(std::shared_ptr<netutils::TcpSocket> listener)
{
	auto client = std::make_shared<netutils::TcpSocket>(netutils::NetworkAddress::IPv4);

	netutils::NetworkAddress address;
	if (listener->accept(*client, address))
	{
		std::cerr << strerror(errno) << std::endl;
	}

	client->setNonBlocking(true);

	epoll_event ev = {
			.events = EPOLLIN,
			.data = { .fd = client->handle() }
	};
	if (epoll_ctl(_epollfd, EPOLL_CTL_ADD, client->handle(), &ev) == -1)
	{
		perror("epoll_ctl: conn_sock");
		exit(EXIT_FAILURE);
	}

	std::cout << "new connection" << std::endl;

	return 0;
}

int EchoServer::process_message(std::string &message)
{
	return 0;
}

int EchoServer::read_message(int fd)
{
	return 0;
}
