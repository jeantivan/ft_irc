#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <sstream>
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
#include <csignal>
#include <fcntl.h>

#include "utils.hpp"
#include "Client.hpp"
#include "Command.hpp"

class Server
{
private:
	std::string port_;
	int listener_; // Socket fd
	std::string password_;
	std::vector<struct pollfd> connections_;
	std::map<int, Client> clients_;

	// Constructors private to avoid duplication of the Server
	Server();
	Server(const Server &other);
	Server &operator=(const Server &other);

	// Flag for a Graceful Shutdown
	static bool signal_received_;

public:
	~Server();

	Server(const char *port, const char *pass);

	std::string getPort() const;
	int getListener() const;
	const std::string &getPassword() const;

	// Bind listener to port
	void init();

	// Main loop to handle connections and data.
	void run();

	// Accept new client connection
	void acceptNewClient();

	// Receive data send by client
	void receiveClientData(size_t client_index);

	// Disconnect Client
	void disconnectClient(size_t client_index);

	// Send client data
	bool sendClientData(size_t client_index);

	// Static signal handler;
	static void signalHandler(int signal);

	// Handle command
	void handleCommand(size_t client_index, const Command &cmd);
};

#endif // SERVER_HPP
