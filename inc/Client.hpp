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
	Client();
	Client(const Client &other);
	Client &operator=(const Client &other);

public:
	~Client();

	Client(int fd, const std::string &ip);

	int getFd() const;
	const std::string &getIp() const;
};

#endif // CLIENT_HPP
