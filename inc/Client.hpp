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

	bool isAuth_;
	std::string nick_;
	std::string realname_;

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

	void appendToReadBuf(const char *data, size_t len);
	void eraseFromWriteBuf(size_t len);
};

#endif // CLIENT_HPP
