#include "Server.hpp"

Server::Server() : port_(-1), socket_(-1), password_("") {}

Server::~Server() {}

Server::Server(const Server &other) : port_(other.port_), socket_(other.socket_), password_(other.password_) {}

Server &Server::operator=(const Server &other)
{
	if (this != &other)
	{
		port_ = other.port_;
		socket_ = other.socket_;
		password_ = other.password_;
	}

	return *this;
}

Server::Server(int p, const char *pass) : port_(p), socket_(-1), password_(pass) {}

// Getters
int Server::getPort() const
{
	return port_;
}

int Server::getSocket() const
{
	return socket_;
}

const std::string &Server::getPassword() const
{
	return password_;
}
