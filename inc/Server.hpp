#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>

class Server
{
private:
	int port_;
	int listener_; // Socket fd
	std::string password_;

public:
	Server();
	Server(const Server &other);
	~Server();
	Server &operator=(const Server &other);

	Server(int port, const char *pass);

	int getPort() const;
	int getListener() const;
	const std::string &getPassword() const;
};

#endif // SERVER_HPP
