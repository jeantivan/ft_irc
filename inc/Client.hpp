#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>
#include <string>
#include <stdexcept>

class Client
{
private:
	int fd;
	std::string ip;
	std::string readBuf_;
	std::string writeBuf_;

public:
	Client();
	Client(const Client &other);
	Client &operator=(const Client &other);
	~Client();

	Client(int fd, const std::string &ip);

	// Getters;
	int getFd() const;
	const std::string &getIp() const;
	const std::string &getReadBuf() const;
	const std::string &getWriteBuf() const;

	bool hasCompleteCommand();
	std::string extractCommand();
};

#endif // CLIENT_HPP
