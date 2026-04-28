#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <string>
#include <stdexcept>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <cerrno>

#include "utils.hpp"

class Server
{
private:
	std::string port_;
	int listener_; // Socket fd
	std::string password_;

public:
	Server();
	Server(const Server &other);
	~Server();
	Server &operator=(const Server &other);

	Server(const char *port, const char *pass);

	std::string getPort() const;
	int getListener() const;
	const std::string &getPassword() const;

	// Bind listener to port
	void bindToPort();
};

#endif // SERVER_HPP
