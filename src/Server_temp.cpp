#include "Server.hpp"
#include "Command/CommandFactory.hpp"
#include "Command/TopicCommand.hpp"
#include "ResponseBuilder.hpp"
#include "Channel.hpp"
#include "Mode/InviteOnlyMode.hpp"
#include "Mode/TopicRestrictedMode.hpp"
#include "Mode/PasswordMode.hpp"
#include "Mode/UserLimitMode.hpp"
#include "Mode/OperatorMode.hpp"

Server::Server() : port_(""), listener_(-1), dummySocket_(-1), password_(""), nameServer_(NAME_SERVER), creationDate_(time(NULL)), checkZombiesDate_(creationDate_ + PERIODICCHECK), connections_(), clients_(), used_nicks_(), _channels_(), modeHandlers_()
{
	modeHandlers_['i'] = new InviteOnlyMode();
	modeHandlers_['t'] = new TopicRestrictedMode();
	modeHandlers_['k'] = new PasswordMode();
	modeHandlers_['l'] = new UserLimitMode();
	modeHandlers_['o'] = new OperatorMode();
}

Server::~Server()
{
	std::cout << "[ircserver]: Shutting down the server" << std::endl;

	for (size_t i = 0; i < connections_.size(); i++)
	{
		int fd = connections_[i].fd;

		if (fd != listener_)
		{
			std::cout << "[ircserver]: Client " << fd << " disconnected gracefully." << std::endl;
			close(fd);
		}
	}

	if (listener_ != -1)
		close(listener_);

	if (dummySocket_ != -1)
		close(dummySocket_);

	used_nicks_.clear();
	delete modeHandlers_['i'];
	delete modeHandlers_['t'];
	delete modeHandlers_['k'];
	delete modeHandlers_['l'];
	delete modeHandlers_['o'];
}

Server::Server(const Server &other) : port_(other.port_), listener_(other.listener_), dummySocket_(other.dummySocket_), password_(other.password_), nameServer_(other.nameServer_), creationDate_(other.creationDate_), checkZombiesDate_(other.checkZombiesDate_) {}

Server &Server::operator=(const Server &other)
{
	if (this != &other)
	{
		port_ = other.port_;
		listener_ = other.listener_;
		dummySocket_ = other.dummySocket_;
		password_ = other.password_;
		nameServer_ = other.nameServer_;
		creationDate_ = other.creationDate_;
		checkZombiesDate_ = other.checkZombiesDate_;
	}

	return *this;
}

Server::Server(const char *port, const char *pass) : port_(port), listener_(-1), dummySocket_(-1), password_(pass), nameServer_(NAME_SERVER), creationDate_(time(NULL)), checkZombiesDate_(creationDate_ + PERIODICCHECK), used_nicks_(), _channels_(), modeHandlers_()
{
	modeHandlers_['i'] = new InviteOnlyMode();
	modeHandlers_['t'] = new TopicRestrictedMode();
	modeHandlers_['k'] = new PasswordMode();
	modeHandlers_['l'] = new UserLimitMode();
	modeHandlers_['o'] = new OperatorMode();

	init();

	struct pollfd listener_poll;
	listener_poll.fd = listener_;
	listener_poll.events = POLLIN;
	listener_poll.revents = 0;

	connections_.push_back(listener_poll);

	std::cout << "ft_irc: listener created with fd " << listener_ << " on port " << port_ << std::endl;
}

// Getters
std::string Server::getPort() const
{
	return port_;
}

int Server::getListener() const
{
	return listener_;
}

const std::string &Server::getPassword() const
{
	return password_;
}

const std::string &Server::getName() const
{
	return nameServer_;
}

size_t Server::findConnectionByFd(int fd) const
{
	for (size_t i = 0; i < connections_.size(); i++)
	{
		if (connections_[i].fd == fd)
			return i;
	}
	return static_cast<size_t>(-1);
}

void Server::init()
{
	int listener;
	int yes = 1;
	int rv;

	struct addrinfo hints, *ai, *p;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if ((rv = getaddrinfo(NULL, port_.c_str(), &hints, &ai)) != 0)
	{
		throw std::runtime_error("getaddrinfo failed " + std::string(gai_strerror(rv)));
	}

	for (p = ai; p != NULL; p = p->ai_next)
	{
		listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);

		if (listener < 0)
			continue;

		setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

		fcntl(listener, F_SETFL, O_NONBLOCK);

		if (bind(listener, p->ai_addr, p->ai_addrlen) < 0)
		{
			close(listener);
			continue;
		}
		break;
	}

	if (p == NULL)
	{
		throw std::runtime_error("failed to bind. Reason: " + std::string(std::strerror(errno)));
	}

	freeaddrinfo(ai);

	if (listen(listener, 10) == -1)
	{
		throw std::runtime_error("failed to listen");
	}

	listener_ = listener;

	dummySocket_ = socket(AF_INET, SOCK_STREAM, 0);

	if (dummySocket_ == -1)
	{
		throw std::runtime_error("failed to open dummy socket");
	}
}

void Server::run()
{
	std::cout << "[ircserver]: Waiting for connections" << std::endl;
	while (Server::signal_received_ == false)
	{
		int pool_count = poll(&connections_[0], connections_.size(), UNBLOCKPOLL);

		if (pool_count == -1)
		{
			throw std::runtime_error("poll failed " + std::string(std::strerror(errno)));
		}

		for (int i = connections_.size() - 1; i >= 0; i--)
		{
			if (connections_[i].revents & (POLLIN | POLLHUP))
			{
				int fd = connections_[i].fd;

				try
				{
					if (fd == listener_)
						acceptNewClient();
					else
						receiveClientData(i);
				}
				catch (const std::exception &e)
				{
					std::cerr << "[ircserver]: Error handling client " << fd << ": " << e.what() << std::endl;

					if (fd == listener_)
						throw;

					disconnectClient(fd);
				}
			}
			else if (connections_[i].revents & POLLOUT)
			{
				if (sendClientData(i))
				{
					std::cerr << "Error: Not all client<" << connections_[i].fd << ", " << clients_[connections_[i].fd].getFd() << "> data could be sent" << std::endl;
				}
			}
		}

		if (time(NULL) > checkZombiesDate_)
		{
			dezombify();
			checkZombiesDate_ = time(NULL) + PERIODICCHECK;
		}
	}
}

void Server::rejectConnection()
{
	if (dummySocket_ != -1)
	{
		close(dummySocket_);
	}

	int overflowFd = accept(listener_, NULL, NULL);
	if (overflowFd != -1)
	{
		close(overflowFd);
	}

	dummySocket_ = socket(AF_INET, SOCK_STREAM, 0);
	if (dummySocket_ == -1)
	{
		throw std::runtime_error("Failed to open dummy socket after rejection");
	}
}

void Server::acceptNewClient()
{
	struct sockaddr_storage remoteaddr;
	socklen_t addrlen = sizeof(remoteaddr);
	int new_fd;

	new_fd = accept(listener_, reinterpret_cast<struct sockaddr *>(&remoteaddr), &addrlen);

	if (new_fd == -1)
	{

		rejectConnection();
		std::cerr << "[ircserver]: accept failed. Possibly FDs limit reached" << std::endl;
		return;
	}

	fcntl(new_fd, F_SETFL, O_NONBLOCK);

	try
	{
		struct pollfd new_connection;

		new_connection.fd = new_fd;
		new_connection.events = POLLIN;
		new_connection.revents = 0;

		connections_.push_back(new_connection);
		std::string remoteIp = getIpStr(reinterpret_cast<struct sockaddr *>(&remoteaddr));

		clients_[new_fd] = Client(new_fd, remoteIp);

		std::cout << "[ircserver]: New connection from " << remoteIp << " on socket " << new_fd << std::endl;
	}
	catch (...)
	{
		clients_.erase(new_fd);
		close(new_fd);
		throw;
	}
}

void Server::receiveClientData(size_t client_index)
{
	char buffer[1024];
	std::memset(buffer, 0, sizeof(buffer));

	int client_fd = connections_[client_index].fd;
	Client &client = clients_[client_fd];
	int bytes_received = recv(client_fd, buffer, sizeof(buffer), 0);
	if (bytes_received <= 0)
	{
		if (bytes_received == 0)
		{
			std::cout << "[ircserver]: Client " << client_fd << " disconnected (Closed client end)" << std::endl; // ¿¿¿Es correcto el mensaje???
		}
		else
		{
			std::cerr << "[ircserver]: recv failed on client " << client_fd << " " << std::strerror(errno) << std::endl;
		}

		disconnectClient(client_fd); // TODO esta dejando enlaces colgantes al cliente desconectado en los canales
		// mejor emular un quitcommand, para eliminar al cliente de los canales
		return;
	}

	std::cout << "[ircserver]: Received " << bytes_received << " bytes from client " << client_fd << std::endl;
	client.appendToReadBuf(buffer, bytes_received);

	// Si acumulamos mas de MAX_READBUF bytes y todavia no hay un "\r\n", la linea rompe el protocolo (cliente roto o ataque).
	if (client.getReadBuf().size() > MAX_READBUF && !client.hasCompleteCommand())
	{
		std::cerr << "[ircserver]: Client " << client_fd << " input line too long (" << client.getReadBuf().size() << " bytes), disconnecting" << std::endl;
		queueClientData(client, "ERROR :Input line too long\r\n");
		client.setToDisconnect();
		return;
	}

	while (client.hasCompleteCommand() && !client.getToDisconnect())
	{
		std::string raw_cmd = client.extractCommand();
		std::string type;
		std::vector<std::string> params;

		if (!parse(raw_cmd, type, params))
		{
			std::cerr << "Bad command" << std::endl;
		}

		CommandFactory factory;

		Command *cmd = factory.createCommand(type, params);
		if (!cmd)
			continue;

		try
		{
			cmd->execute(&client, this);
		}
		catch (...)
		{
			delete cmd;
			throw;
		}
		delete cmd;
	}
}

void Server::disconnectClient(int fd)
{
	std::map<int, Client>::iterator clientIt = clients_.find(fd);
	if (clientIt != clients_.end())
	{
		Client &client = clientIt->second;

		std::string nick = client.getNick();
		if (!nick.empty())
		{
			removeNick(nick);
		}

		std::string quitMsg = ":" + client.getNick() + "!" + client.getUser() + "@" + client.getIp() + " QUIT :Connection reset\r\n";
		std::vector<std::string> channelsToRemove;

		for (std::map<std::string, Channel>::iterator it = _channels_.begin(); it != _channels_.end(); ++it)
		{
			Channel &channel = it->second;
			if (channel.isMember(fd))
			{
				channel.removeClient(fd);
				channel.broadcastAll(quitMsg, this);

				if (channel.isEmpty())
					channelsToRemove.push_back(it->first);
			}
		}

		for (size_t i = 0; i < channelsToRemove.size(); ++i)
		{
			_channels_.erase(channelsToRemove[i]);
			std::cout << "[ircserver]: Channel " << channelsToRemove[i] << " deleted during disconnect (no members left)." << std::endl;
		}
	}

	close(fd);

	clients_.erase(fd);

	for (size_t i = 0; i < connections_.size(); i++)
	{
		if (connections_[i].fd == fd)
		{
			connections_.erase(connections_.begin() + i);
			break;
		}
	}
	std::cout << "[ircserver]: Client " << fd << " disconnected (The server did it)" << std::endl;
}


void Server::requestRegistration(Client &client)
{
	if (client.getState() == AUTH_COMPLETE)
	{
		ResponseBuilder response;
		if (password_ == client.getPassword())
		{
			std::cout << "[ircserver]: Client <" << client.getFd() << ", " << client.getIp() << "> is authenticated" << std::endl;
			client.setAuth(true);

			// TODO:extraer este bloque a funcion auxiliar WelcomReply()
			// 001    RPL_WELCOME	"Welcome to the Internet Relay Network <nick>!<user>@<host>"
			response.prefix(getName()).numeric(1).target(client.getNick()).trailing("Welcome to the Internet Relay Network " + client.getNick() + "!" + client.getUser() + "@" + client.getIp());
			queueClientData(client, response.build());
			// 002	YOURHOST		"Your host is <servername>, running version <ver>"
			response.numeric(2).trailing("Your host is " + getName() + ", running version " + SERVER_VERSION);
			queueClientData(client, response.build());
			// 003    RPL_CREATED	"This server was created <date>"
			char date[64];
			struct tm *tm_info = localtime(&creationDate_);
			strftime(date, sizeof(date), "%c", tm_info);
			std::string createdMsg = "This server was created ";
			createdMsg += date;
			response.numeric(3).trailing(createdMsg);
			queueClientData(client, response.build());
			// 004    RPL_MYINFO		"<servername> <version> <available user modes> <available channel modes>"
			response.numeric(4).trailing(nameServer_ + " " + SERVER_VERSION + " " + "io itkol");
			queueClientData(client, response.build());
			// fin WelcomReply()

			return;
		}
		else
		{
			response.prefix(getName()).numeric(ERR_PASSWDMISMATCH).target(client.getNick()).trailing("Password incorrect");
			queueClientData(client, response.build());

			std::cerr << "[ircserver]--->" << client.getFd() << " Error: Password incorrect" << std::endl;
			client.setToDisconnect();
		}
	}
}

bool Server::sendClientData(size_t client_index)
{
	int fd = connections_[client_index].fd;
	Client &client = clients_[fd];
	const std::string &clientWriteBuf = client.getWriteBuf();

	if (clientWriteBuf.empty())
	{
		connections_[client_index].events &= ~POLLOUT;
		std::cerr << "[INFO] client in socket:" << fd << "calls sendClientData with writeBuff_ empty." << std::endl;
		return false;
	}

	ssize_t bytes_sent = send(fd, clientWriteBuf.c_str(), clientWriteBuf.size(), 0);

	if (bytes_sent > 0)
	{
		client.eraseFromWriteBuf(bytes_sent);
		if (client.getWriteBuf().empty())
		{
			connections_[client_index].events &= ~POLLOUT;
			if (client.getToDisconnect())
				disconnectClient(fd);
		}
	}
	else if (bytes_sent == -1)
	{
		disconnectClient(fd);
		return true;
	}
	else
		std::cerr << "[INFO] client in shocket:" << fd << "send() return 0;" << std::endl;
	return false;
}

void Server::queueClientData(Client &client, const std::string &data)
{
	size_t id = findConnectionByFd(client.getFd());
	if (id == static_cast<size_t>(-1))
	{
		std::cerr << "ERROR en findConnectionByFd" << std::endl;
		return;
	}
	connections_[id].events |= POLLOUT;
	if (!client.getToDisconnect())
		client.appendToWriteBuf(data);
}

// NICK COMMAND
bool Server::isNickInUse(const std::string &nick) const
{
	return used_nicks_.count(nick) > 0;
}

void Server::addNick(const std::string &nick)
{
	used_nicks_.insert(nick);
}

void Server::removeNick(const std::string &nick)
{
	used_nicks_.erase(nick);
}

// JOIN COMMAND
bool Server::isAchannel(const std::string &channel) const
{
	if (_channels_.find(channel) == _channels_.end())
		return false;
	else
		return true;
}

Channel *Server::getChannel(const std::string &name)
{
	if (isAchannel(name))
		return &(_channels_[name]);
	return NULL;
}

std::map<std::string, Channel> &Server::getChannels()
{
	return _channels_;
}

bool Server::joinChannel(Client *client, const std::string &nameChannel, const std::string &password)
{
	Channel *channel;
	std::string nickList;
	ResponseBuilder response;
	int clientFd = client->getFd();
	std::string clientNick = client->getNick();

	if (isAchannel(nameChannel))
	{
		channel = &(_channels_[nameChannel]);
		if (channel->getMembers().find(clientFd) != channel->getMembers().end())
		{
			std::cout << "[ircserver]:" << clientNick << "send JOIN->"
					  << nameChannel << ". But he was already in the channel" << std::endl;
			return false;
		}
		else
		{
			if (channel->getPassword().compare(password))
			{
				sendNumericReply(client, ERR_BADCHANNELKEY, nameChannel, "Cannot join channel (+k)");
				std::cout << "[ircserver]:" << clientNick << "send JOIN->"
						  << nameChannel << ". But bad passkey" << std::endl;
				return false;
			}
			if (channel->isInviteOnly())
			{
				if (!channel->isInvited(clientFd))
				{
					sendNumericReply(client, ERR_INVITEONLYCHAN, nameChannel, "Cannot join channel (+i)");
					return false;
				}
			}
			if (channel->getUserLimit() != 0 &&
				channel->getUserLimit() <= channel->getMembers().size())
			{
				std::cout << "[ircserver]:" << clientNick << "send JOIN->"
						  << nameChannel << ". But Channel is full" << std::endl;
				sendNumericReply(client, ERR_CHANNELISFULL, nameChannel, "Cannot join channel (channel is full)");
				return false;
			}

			if (channel->getMembers().size() >= MAX_CHANNEL_MEMBERS)
			{
				std::cout << "[ircserver]:" << clientNick << "send JOIN->"
						  << nameChannel << ". But Channel is full" << std::endl;
				sendNumericReply(client, ERR_CHANNELISFULL, nameChannel, "Cannot join channel (channel is full)");
				return false;
			}

			channel->addClient(client);
			std::cout << "[ircserver]: " << clientNick << " Join to: " << nameChannel << std::endl;
		}
	}
	else
	{
		channel = &createChannel(nameChannel);
		channel->addClient(client);
		channel->addOperator(clientFd);
	}
	channel->broadcastAll(":" + client->getPrefix() + " JOIN " + nameChannel + "\r\n", this);
	std::vector<std::string> topicVect;
	topicVect.push_back(nameChannel);
	TopicCommand topic("TOPIC", topicVect);
	topic.execute(client, this);
	namreply(client, channel);
	return true;
}

void Server::sendNumericReply(Client *client, int numeric, const std::string &params, const std::string &trailing)
{
	ResponseBuilder response;

	response.prefix(getName())
		.numeric(numeric)
		.target(client->getNick());

	if (!params.empty())
		response.params(params);

	if (!trailing.empty())
	{
		response.trailing(trailing);
	}

	queueClientData(*client, response.build());
}

void Server::namreply(Client *client, Channel *channel)
{
	std::istringstream nicksStream(channel->getNickList()); // la nicklist deberia incluir "@" delante de cada operador obtener el FD apartir de un nick en este punto del codigo es un dolor
	std::ostringstream paqNicks;
	std::string nick;
	int i = 0;

	while (nicksStream >> nick)
	{
		if (i > 0)
		{
			paqNicks << " ";
		}
		paqNicks << nick;
		i++;

		//	Why 35 nicknames per RPL_NAMREPLY?
		//	Maximum message length: 512
		//	Each nickname is at most 9 characters + @ + " " = 11
		//	(prefix + cmd + (11 * 40) + \r\n) = 512
		//	Since there is no need to push the protocol limit to the max,
		//	we use a limit of 35 nicknames per response instead of 40.
		if (i == 35)
		{
			sendNumericReply(client, RPL_NAMREPLY, "= " + channel->getName(), paqNicks.str()); // OJO!!! cuando se implementen los modos gestionar "= "

			paqNicks.str("");
			paqNicks.clear();
			i = 0;
		}
	}

	if (!paqNicks.str().empty())
	{
		sendNumericReply(client, RPL_NAMREPLY, "= " + channel->getName(), paqNicks.str()); // OJO!!! cuando se implementen los modos gestionar "= "
	}

	// Fin del protocolo (RPL_ENDOFNAMES) usando la MISMA función genérica
	sendNumericReply(client, RPL_ENDOFNAMES, channel->getName(), "End of /NAMES list");
}

Channel &Server::createChannel(const std::string &name)
{
	if (_channels_.find(name) != _channels_.end())
		std::cerr << "[ircserver] createChannel: " << name << "cannot be created, it already exists.";
	else
	{
		_channels_[name] = Channel(name);

		std::cout << "[ircserver]: Channel " << name << " created" << std::endl;
	}

	return _channels_[name];
}
Client *Server::findClientByNick(const std::string &nick_to_find)
{
	if (nick_to_find.empty())
		return NULL;

	std::map<int, Client>::iterator it;

	for (it = clients_.begin(); it != clients_.end(); ++it)
	{
		if (it->second.getNick() == nick_to_find)
		{
			return &(it->second);
		}
	}
	return NULL;
}

// PART COMMMAND
void Server::leaveChannel(Client *client, const std::string &nameChannel, const std::string &reason)
{
	ResponseBuilder response;
	int clientFd = client->getFd();

	Channel *channelPtr = getChannel(nameChannel);
	if (!channelPtr)
	{
		response.prefix(getName())
			.numeric(ERR_NOSUCHCHANNEL)
			.target(client->getNick())
			.trailing(nameChannel + " :No such channel");
		queueClientData(*client, response.build());
		std::cerr << "[ircserver]: Error: ERR_NOSUCHCHANNEL para " << nameChannel << std::endl;
		return;
	}

	Channel &channel = *channelPtr;

	if (!channel.isMember(clientFd))
	{
		response.prefix(getName())
			.numeric(ERR_NOTONCHANNEL)
			.target(client->getNick())
			.params(nameChannel)
			.trailing("You're not on that channel");
		queueClientData(*client, response.build());
		std::cerr << "[ircserver]: Error: ERR_NOTONCHANNEL en " << nameChannel << std::endl;
		return;
	}

	std::string partMsg = ":" + client->getNick() + "!" + client->getUser() + "@" + client->getIp() + " PART " + nameChannel;
	if (!reason.empty())
		partMsg += " :" + reason;
	partMsg += "\r\n";

	channel.broadcastAll(partMsg, this);

	channel.removeClient(clientFd);
	std::cout << "[ircserver]: " << client->getNick() << " leave " << nameChannel << "reason" << reason << std::endl;

	if (channel.isEmpty())
	{
		_channels_.erase(nameChannel);
		std::cout << "[ircserver]: Channel " << nameChannel << " deleted de _channels_ (no members left)." << std::endl;
	}
}

void Server::dezombify()
{
	std::map<int, Client>::iterator it = clients_.begin();
	while (it != clients_.end())
	{
		int fd = it->first;
		bool zombie = it->second.getToDisconnect() && it->second.getToDisconnectSince() + TEARDOWNTIMEMAX < time(NULL);
		++it;

		if (zombie)
			disconnectClient(fd);
	}
}
// TODO: PASAR A UN ARCHIVO NUEVO DENTRO DE src/Server/Modes
ModeHandler *Server::getModeHandler(const char &mode) const
{
	std::map<char, ModeHandler *>::const_iterator it = modeHandlers_.find(mode);

	if (it != modeHandlers_.end())
	{
		return it->second;
	}

	return NULL;
}
