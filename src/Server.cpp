#include "Server.hpp"

Server::Server() : port_(-1), listener_(-1), password_("") {}

Server::~Server() {}

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

Server::Server(int p, const char *pass) : port_(p), listener_(-1), password_(pass)
{
}

// Getters
int Server::getPort() const
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
