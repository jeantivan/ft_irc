#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <string>
#include <stdexcept>
#include <vector>
#include <map>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <poll.h>

#include "utils.hpp"

class Server
{
private:
	std::string port_;
	int listener_; // Socket fd
	std::string password_;
	std::vector<struct pollfd> connections_;
	// TODO: Cambiar std::string por la futura clase Client;
	std::map<int, std::string> clients_;

	// Constructors private to avoid duplication of the Server
	Server();
	Server(const Server &other);
	Server &operator=(const Server &other);

public:
	~Server();

	Server(const char *port, const char *pass);

	std::string getPort() const;
	int getListener() const;
	const std::string &getPassword() const;

	// Bind listener to port
	void init();
};

#endif // SERVER_HPP
