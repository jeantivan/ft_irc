#include "Client.hpp"

Client::Client() : fd(-1), ip(""), readBuf_(""), writeBuf_("") {}

Client::Client(const Client &other) : fd(other.fd), ip(other.ip), readBuf_(other.readBuf_), writeBuf_(other.writeBuf_)  {}

Client &Client::operator=(const Client &other)
{
	if (this != &other)
	{
		fd = other.fd;
		ip = other.ip;
		readBuf_ = other.readBuf_;
		writeBuf_ = other.writeBuf_;
	}

	return *this;
}

Client::~Client() {}

Client::Client(int fd, const std::string &ip) : fd(fd), ip(ip), readBuf_(""), writeBuf_("") {}

int Client::getFd() const
{
	return fd;
}

const std::string &Client::getIp() const
{
	return ip;
}

const std::string &Client::getReadBuf() const {
	return readBuf_;
}

const std::string &Client::getWriteBuf() const {
	return writeBuf_;
}

bool Client::hasCompleteCommand() {
	return readBuf_.find("\r\n") != std::string::npos;
}

std::string Client::extractCommand() {
	size_t delim = readBuf_.find("\r\n");

	if (delim == std::string::npos)
		return "";

	std::string cmd = readBuf_.substr(0, delim + 2);
	readBuf_ = readBuf_.substr(delim + 2);

	return cmd;
}

void Client::appendToReadBuf_(const char *data, size_t len) {
	readBuf_.append(data, len);
}
