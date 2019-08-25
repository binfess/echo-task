#include "echoserver.hpp"

#include <regex>
#include <unistd.h>
#include <cstring>
#include <cctype>

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

void EchoServer::run()
{
	if (_epollfd == -1)
	{
		return;
	}

	init_tcp();
	init_udp();
	event_loop();
}

void EchoServer::init_tcp()
{
	_tcp_listener->setSocketOption(netutils::TcpSocket::ReuseAddressOption, 1);
	_tcp_listener->setNonBlocking(true);

	if (_tcp_listener->bind(netutils::NetworkAddress(netutils::NetworkAddress::IPv4, "0.0.0.0", 45000)))
	{
		throw std::runtime_error(std::string("init_tcp: ") + strerror(errno));
	}

	if (_tcp_listener->listen(10))
	{
		throw std::runtime_error(std::string("init_tcp: ") + strerror(errno));
	}

	_listenfd = _tcp_listener->handle();
	epoll_event ev = {
			.events = EPOLLIN,
			.data = { .fd = _listenfd }
	};

	if (epoll_ctl(_epollfd, EPOLL_CTL_ADD, _listenfd, &ev))
	{
		throw std::runtime_error(std::string("init_tcp: ") + strerror(errno));
	}
}

void EchoServer::init_udp()
{
	_udp_socket->setNonBlocking(true);
	if (_udp_socket->bind(netutils::NetworkAddress(netutils::NetworkAddress::IPv4, "0.0.0.0", 45000)))
	{
		throw std::runtime_error(std::string("init_udp: ") + strerror(errno));
	}

	_udpfd = _udp_socket->handle();
	epoll_event ev = {
			.events = EPOLLIN,
			.data = { .fd = _udpfd }
	};
	if (epoll_ctl(_epollfd, EPOLL_CTL_ADD, _udpfd, &ev) == -1)
	{
		throw std::runtime_error(std::string("init_udp: ") + strerror(errno));
	}
}

void EchoServer::event_loop()
{
	epoll_event events[MAX_EPOLL_EVENTS];
	for (;;)
	{
		int nfds = epoll_wait(_epollfd, events, MAX_EPOLL_EVENTS, -1);
		if (nfds == -1)
		{
			throw std::runtime_error(std::string("event_loop: ") + strerror(errno));
		}

		for (int n = 0; n < nfds; ++n)
		{
			if ((events[n].events & EPOLLERR) || (events[n].events & EPOLLHUP))
			{
				/* close connection */
				_tcp_clients.erase(events->data.fd);
				continue;
			}

			if (events[n].data.fd == _listenfd)
			{
				accept_connection();
			}
			else if (events[n].data.fd == _udpfd)
			{
				if (events[n].events & EPOLLOUT)
				{
					write_udp_echo();
				}

				if (events[n].events & EPOLLIN)
				{
					read_udp_echo();
				}
			}
			else
			{
				if (events[n].events & EPOLLOUT)
				{
					write_tcp_echo(events[n].data.fd);
				}

				if (events[n].events & EPOLLIN)
				{
					read_tcp_echo(events[n].data.fd);
				}
			}
		}
	}
}

void EchoServer::accept_connection()
{
	auto client = std::make_shared<netutils::TcpSocket>(netutils::NetworkAddress::IPv4);

	netutils::NetworkAddress address;
	if (_tcp_listener->accept(*client, address))
	{
		std::cerr << "accept_connection: " << strerror(errno) << std::endl;
		return;
	}

	client->setNonBlocking(true);

	epoll_event ev = {
			.events = EPOLLIN,
			.data = { .fd = client->handle() }
	};
	if (epoll_ctl(_epollfd, EPOLL_CTL_ADD, client->handle(), &ev) == -1)
	{
		throw std::runtime_error(std::string("accept_connection: ") + strerror(errno));
	}

	_tcp_clients.emplace(client->handle(), echo_client_t(client));
}

void EchoServer::process_message(std::string &message)
{
	std::cout << "message: " << message << std::endl;

	/* find numbers in range 0...9 */
	std::vector<int> numbers;

	for (int i = 0; i < message.size(); i++)
	{
		if ((i == 0 || !std::isdigit(message[i - 1])) && std::isdigit(message[i]) &&
		    (i == message.size() - 1 || !std::isdigit(message[i + 1])))
		{
			numbers.emplace_back(message[i] - '0');
		}
	}

	if (!numbers.size())
	{
		std::cout << "failed to find numbers" << std::endl;
		return;
	}

	/* calc sum  */
	int sum = 0;
	std::for_each(std::begin(numbers), std::end(numbers), [&sum] (int number) { sum += number; });

	std::cout << "sum " << sum << std::endl;

	/* sort numbers */
	std::sort(std::begin(numbers), std::end(numbers), std::greater<int>());

	/* get min and max values */
	int max = numbers.front(), min = numbers.back();

	std::cout << "max " << max << " min " << min << std::endl;

}

void EchoServer::read_tcp_echo(int fd)
{
	static std::array<std::uint8_t, 65507> buf;
	auto &client = _tcp_clients[fd].sock;

	size_t len = buf.size();
	if (client->recv(buf.data(), len))
	{
		std::cerr << "read_tcp_echo: " << strerror(errno) << std::endl;
		return;
	}

	std::string message((char*) buf.data(), len);

	process_message(message);

	_tcp_clients[fd].requests.emplace_back(0, std::move(message));

	epoll_event ev = {
			.events = EPOLLIN | EPOLLOUT,
			.data = { .fd = client->handle() }
	};
	if (epoll_ctl(_epollfd, EPOLL_CTL_MOD, client->handle(), &ev) == -1)
	{
		throw std::runtime_error(std::string("read_tcp_echo: ") + strerror(errno));
	}
}

void EchoServer::write_tcp_echo(int fd)
{
	auto &client = _tcp_clients[fd].sock;

	auto &sent_bytes = _tcp_clients[fd].requests.front().first;
	std::string &message = _tcp_clients[fd].requests.front().second;

	auto remain_bytes = message.size() - sent_bytes;
	if (client->send(message.data() + sent_bytes, remain_bytes) == -1)
	{
		std::cerr << "write_tcp_echo: " << strerror(errno) << std::endl;
		return;
	}

	sent_bytes += remain_bytes;
	if (sent_bytes == message.size())
	{
		/* message was sent */
		_tcp_clients[fd].requests.pop_front();
	}

	epoll_event ev = {
			.events = EPOLLIN,
			.data = { .fd = client->handle() }
	};
	if (epoll_ctl(_epollfd, EPOLL_CTL_MOD, client->handle(), &ev) == -1)
	{
		throw std::runtime_error(std::string("write_tcp_echo: ") + strerror(errno));
	}
}

void EchoServer::read_udp_echo()
{
	netutils::NetworkAddress peer;
	static std::array<std::uint8_t, 65507> buf;

	size_t len = buf.size();
	if (_udp_socket->recvFrom(buf.data(), len, peer))
	{
		std::cerr << "read_udp_echo: " << strerror(errno) << std::endl;
		return;
	}

	std::string message((char*) buf.data(), len);

	process_message(message);

	_udp_requests.emplace_back(0, std::move(message), std::move(peer));

	epoll_event ev = {
			.events = EPOLLIN | EPOLLOUT,
			.data = { .fd = _udp_socket->handle() }
	};
	if (epoll_ctl(_epollfd, EPOLL_CTL_MOD, _udp_socket->handle(), &ev) == -1)
	{
		throw std::runtime_error(std::string("read_udp_echo: ") + strerror(errno));
	}
}

void EchoServer::write_udp_echo()
{
	auto &req_tuple = _udp_requests.front();

	auto &sent_bytes = std::get<0>(req_tuple);
	std::string &message = std::get<1>(req_tuple);
	auto &peer = std::get<2>(req_tuple);

	auto remain_bytes = message.size() - sent_bytes;
	if (_udp_socket->sendTo(message.data() + sent_bytes, remain_bytes, peer) == -1)
	{
		std::cerr << "write_tcp_echo: " << strerror(errno) << std::endl;
		return;
	}

	sent_bytes += remain_bytes;
	if (sent_bytes == message.size())
	{
		/* message was sent */
		_udp_requests.pop_front();
	}

	epoll_event ev = {
			.events = EPOLLIN,
			.data = { .fd = _udp_socket->handle() }
	};
	if (epoll_ctl(_epollfd, EPOLL_CTL_MOD, _udp_socket->handle(), &ev) == -1)
	{
		throw std::runtime_error(std::string("write_tcp_echo: ") + strerror(errno));
	}
}
