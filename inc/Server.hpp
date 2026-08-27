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
#include "Channel.hpp"

#include "Mode/ModeHandler.hpp"

#define NAME_SERVER "IRC_Serv" // maximo 9 caracteres (RPL_s construidas con esa convencion)
#define SERVER_VERSION "Beta"
#define MAX_CHANNEL_MEMBERS 200
#define UNBLOCKPOLL 10000 // tiempo en milisegundos que tarda poll en desbloquearse cuando no hay actividad
#define PERIODICCHECK 10
#define TEARDOWNTIMEMAX 20 //cuando un cliente es marcado toDisconnect, y supera este periodo de gracia, sera desconectado aun cuando quedasen datos sin enviar en su buffer de salida

class Channel;
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
	std::map<std::string, Channel> _channels_;
	time_t checkZombiesDate_;

	// ModeHandler
	std::map<char, ModeHandler *> modeHandlers_;

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

	// Respuestas listas y encoladas en una sola funcion.
	// Istancia un objeto ResponseBuilder con los parametros recibidos y lo pone en cola del writeBuf del cliente
	void sendNumericReply(Client *client, int numeric, const std::string &params, const std::string &trailing);

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

	// _Chanels_
	bool isAchannel(const std::string &name) const;
	Channel *getChannel(const std::string &name);
	Channel &createChannel(const std::string &name);
	bool joinChannel(Client *client, const std::string &name, const std::string &password);
	void leaveChannel(Client *client, const std::string &name, const std::string &reason);
	std::map<std::string, Channel> &getChannels();

	// Lanza uno o mas RPL_NAMEREPLY y un y RPL_ENDOFNAMES al final
	void namreply(Client *client, Channel *channel);

	// PrivMsg Command
	Client *findClientByNick(const std::string &nick_to_find);
	
	void dezombify();

	// ModeHandler
	ModeHandler *getModeHandler(const char &mode) const;
};

#endif // SERVER_HPP
