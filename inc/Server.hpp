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

#define NAME_SERVER "IRC_Serv" // Maximum of 9 characters (RPL_s built using that convention)
#define SERVER_VERSION "Beta"
#define MAX_CHANNEL_MEMBERS 200
#define MAX_READBUF 512	  // A larger line is probably trying to overflow `writeBuf_`
#define UNBLOCKPOLL 10000 // The time, in milliseconds, it takes for poll to unlock when there is no activity
#define PERIODICCHECK 10
#define TEARDOWNTIMEMAX 20 // When a client is marked as “toDisconnect” and exceeds this grace period, it will be disconnected even if there is data remaining in its output buffer.

class Channel;
class Server
{
private:
	std::string port_;
	int listener_;	  // Socket fd
	int dummySocket_; // To reject connections when open fd limits are reached
	std::string password_;
	std::string nameServer_;
	time_t creationDate_;
	time_t checkZombiesDate_;
	std::vector<struct pollfd> connections_;
	std::map<int, Client> clients_;
	std::set<std::string> used_nicks_;
	std::map<std::string, Channel> _channels_;

	// ModeHandler
	std::map<char, ModeHandler *> modeHandlers_;

	// Constructors private to avoid duplication of the Server
	Server();
	Server(const Server &other);
	Server &operator=(const Server &other);

	// Flag for a Graceful Shutdown
	static volatile sig_atomic_t signal_received_;

public:
	~Server();

	Server(const char *port, const char *pass);

	std::string getPort() const;
	int getListener() const;
	const std::string &getPassword() const;
	const std::string &getName() const;

	// Find the element in `connections_` to which an FD belongs
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

	// Sends data to the client's output buffer and triggers the POLLOUT event for that client
	void queueClientData(Client &client, const std::string &data);

	/* Responses prepared and queued in a single function.
     Instantiates a ResponseBuilder object with the received parameters and adds it to the client's writeBuf queue*/
	void sendNumericReply(Client *client, int numeric, const std::string &params, const std::string &trailing);

	// Send the data stored in the output buffer using the previous function
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

	// Insert one or more RPL_NAMEREPLY statements and one RPL_ENDOFNAMES statement at the end
	void namreply(Client *client, Channel *channel);

	// PrivMsg Command
	Client *findClientByNick(const std::string &nick_to_find);

	void dezombify();

	// ModeHandler
	ModeHandler *getModeHandler(const char &mode) const;

	void rejectConnection();
};

#endif // SERVER_HPP
