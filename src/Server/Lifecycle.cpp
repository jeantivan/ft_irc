#include "Server.hpp"
#include "Mode/InviteOnlyMode.hpp"
#include "Mode/TopicRestrictedMode.hpp"
#include "Mode/PasswordMode.hpp"
#include "Mode/UserLimitMode.hpp"
#include "Mode/OperatorMode.hpp"

/**
 *
 * No implemented because Sever can't be copied and has only one valid constructor
 *
 * Server::Server() {}
 * Server::Server(const Server &other) {}
 * Server &Server::operator=(const Server &other) {}
 */

Server::Server(const char *port, const char *pass) : port_(port), listener_(-1), dummySocket_(-1), password_(pass), nameServer_(NAME_SERVER), creationDate_(time(NULL)), checkZombiesDate_(creationDate_ + PERIODICCHECK)
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

	delete modeHandlers_['i'];
	delete modeHandlers_['t'];
	delete modeHandlers_['k'];
	delete modeHandlers_['l'];
	delete modeHandlers_['o'];
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

volatile sig_atomic_t Server::signal_received_ = false;

void Server::signalHandler(int signal)
{
	(void)signal;
	Server::signal_received_ = true;
}

ModeHandler *Server::getModeHandler(const char &mode) const
{
	std::map<char, ModeHandler *>::const_iterator it = modeHandlers_.find(mode);

	if (it != modeHandlers_.end())
	{
		return it->second;
	}

	return NULL;
}
