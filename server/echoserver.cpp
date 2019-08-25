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

void EchoServer::run()
{
	if (_epollfd == -1)
	{
		return;
	}

	init_loop();
	event_loop();
}

void EchoServer::init_loop()
{
	_tcp_listener = std::make_shared<netutils::TcpSocket>(netutils::NetworkAddress::IPv4);
	_tcp_listener->setSocketOption(netutils::TcpSocket::ReuseAddressOption, 1);
	_tcp_listener->setNonBlocking(true);

	if (_tcp_listener->bind(netutils::NetworkAddress(netutils::NetworkAddress::IPv4, "0.0.0.0", 45000)))
	{
		throw std::runtime_error(std::string("init_loop: ") + strerror(errno));
	}

	if (_tcp_listener->listen(10))
	{
		throw std::runtime_error(std::string("init_loop: ") + strerror(errno));
	}

	_listenfd = _tcp_listener->handle();
	epoll_event ev = {
			.events = EPOLLIN,
			.data = { .fd = _listenfd }
	};

	if (epoll_ctl(_epollfd, EPOLL_CTL_ADD, _listenfd, &ev))
	{
		throw std::runtime_error(std::string("init_loop: ") + strerror(errno));
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
			else
			{
				if (events[n].events & EPOLLOUT)
				{
					write_message(events[n].data.fd);
				}

				if (events[n].events & EPOLLIN)
				{
					read_message(events[n].data.fd);
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
	std::cout << message;
}

void EchoServer::read_message(int fd)
{
	static std::array<std::uint8_t, 65507> buf;
	auto client = std::dynamic_pointer_cast<netutils::TcpSocket>(_tcp_clients[fd].sock);

	size_t len = buf.size();
	if (client->recv(buf.data(), len))
	{
		std::cerr << "read_message: " << strerror(errno) << std::endl;
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
		throw std::runtime_error(std::string("read_message: ") + strerror(errno));
	}
}

void EchoServer::write_message(int fd)
{
	auto client = std::dynamic_pointer_cast<netutils::TcpSocket>(_tcp_clients[fd].sock);

	auto &sent_bytes = _tcp_clients[fd].requests.front().first;
	std::string &message = _tcp_clients[fd].requests.front().second;

	auto remain_bytes = message.size() - sent_bytes;
	if (client->send(message.data() + sent_bytes, remain_bytes) == -1)
	{
		std::cerr << "write_message: " << strerror(errno) << std::endl;
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
		throw std::runtime_error(std::string("write_message: ") + strerror(errno));
	}
}
