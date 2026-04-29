#include "Server.hpp"

Server::Server() : port_(""), listener_(-1), password_("") {}

Server::~Server()
{
	if (listener_ != -1)
		close(listener_);
}

Server::Server(const Server &other) : port_(other.port_), listener_(other.listener_), password_(other.password_) {}

Server &Server::operator=(const Server &other)
{
	if (this != &other)
	{
		port_ = other.port_;
		listener_ = other.listener_;
		password_ = other.password_;
	}

	return *this;
}

Server::Server(const char *port, const char *pass) : port_(port), listener_(-1), password_(pass)
{
	init();

	struct pollfd listener_poll;
	listener_poll.fd = listener_;
	listener_poll.events = POLLIN;
	listener_poll.revents = 0;

	connections_.push_back(listener_poll);

	std::cout << "ft_irc: listener created with fd " << listener_ << " on port " << port_ << std::endl;
}

// Getters
std::string Server::getPort() const
{
	return port_;
}

int Server::getListener() const
{
	return listener_;
}

const std::string &Server::getPassword() const
{
	return password_;
}

void Server::init()
{
	int listener;
	int yes = 1;
	int rv;

	struct addrinfo hints, *ai, *p;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if ((rv = getaddrinfo(NULL, port_.c_str(), &hints, &ai)) != 0)
	{
		throw std::runtime_error("getaddrinfo failed " + std::string(gai_strerror(rv)));
	}

	for (p = ai; p != NULL; p = p->ai_next)
	{
		listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);

		if (listener < 0)
			continue;

		setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

		if (bind(listener, p->ai_addr, p->ai_addrlen) < 0)
		{
			close(listener);
			continue;
		}
		break;
	}

	if (p == NULL)
	{
		throw std::runtime_error("failed to bind. Reason: " + std::string(std::strerror(errno)));
	}

	freeaddrinfo(ai);

	if (listen(listener, 10) == -1)
	{
		throw std::runtime_error("failed to listen");
	}

	listener_ = listener;
}

void Server::run()
{
	std::cout << "[ircserver]: Waiting for connections" << std::endl;
	while (true)
	{
		int pool_count = poll(&connections_[0], connections_.size(), -1);

		if (pool_count == -1)
		{
			throw std::runtime_error("poll failed " + std::string(std::strerror(errno)));
		}

		for (size_t i = 0; i < connections_.size(); i++)
		{
			if (connections_[i].fd == listener_)
			{
				acceptNewClient();
			}
			else
			{
				std::cout << "New client trying to send data" << std::endl;
			}
		}
	}
}

void Server::acceptNewClient()
{
	std::cout << "WIP..." << std::endl;
}

void Server::receiveClientData(int client_socket)
{
	(void)client_socket;
	std::cout << "WIP..." << std::endl;
}
