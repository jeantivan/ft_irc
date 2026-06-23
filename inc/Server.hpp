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
#include <set>

#include "utils.hpp"
#include "Client.hpp"
#include "Command/Command.hpp"
#include <ctime>

#define NAME_SERVER "Our_IRC_Serv"
#define SERVER_VERSION "Beta"

class Server
{
private:
	std::string port_;
	int listener_; // Socket fd
	std::string password_;
	std::string nameServer_;
	time_t creationDate_;
	std::vector<struct pollfd> connections_;
	std::map<int, Client> clients_;
	std::set<std::string> used_nicks_;

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
	const std::string &getName() const;

	// Busca a que elemento en connections_ pertenece un fd
	size_t findConnectionByFd(int fd) const;

	// Bind listener to port
	void init();

	// Main loop to handle connections and data.
	void run();

	// Accept new client connection
	void acceptNewClient();

	// Receive data send by client
	void receiveClientData(size_t client_index);

	// Disconnect Client
	void disconnectClient(int fd);

	void requestRegistration(Client &client);

	// Envia datos al bufer de salida de client, activa el evento POLLOUT para ese cliente
	void queueClientData(Client &client, const std::string &data);

	// Send client data, los datos almacendos en el buffer desalida con la funcion anterior
	bool sendClientData(size_t client_index);

	// Static signal handler;
	static void signalHandler(int signal);

	// Handle command
	void handleCommand(size_t client_index, const Command &cmd);

	// Nick cmmand
	bool isNickInUse(const std::string &nick) const;
	void addNick(const std::string &nick);
	void removeNick(const std::string &nick);

	// PrivMsg Command
	Client *findClientByNick(const std::string &nick_to_find);
};

#endif // SERVER_HPP
