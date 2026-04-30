#include "Client.hpp"

Client::Client() : fd(-1), ip("") {}

Client::Client(const Client &other) : fd(other.fd), ip(other.ip) {}

Client &Client::operator=(const Client &other)
{
	if (this != &other)
	{
		fd = other.fd;
		ip = other.ip;
	}

	return *this;
}

Client::~Client() {}

Client::Client(int fd, const std::string &ip) : fd(fd), ip(ip) {}

int Client::getFd() const
{
	return fd;
}

const std::string &Client::getIp() const
{
	return ip;
}
