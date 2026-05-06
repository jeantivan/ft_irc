#include "Server.hpp"

Server::Server() : port_(""), listener_(-1), password_("") {}

Server::~Server()
{
	std::cout << "[ircserver]: Shutting down the server" << std::endl;

	for (size_t i = 0; i < connections_.size(); i++)
	{
		int fd = connections_[i].fd;

		if (fd != listener_)
		{
			std::string bye = "Error: Server is shutting down. Goodbye!\r\n";
			send(fd, bye.c_str(), bye.size(), 0);
			close(fd);

			std::cout << "[ircserver]: Client " << fd << " disconnected gracefully." << std::endl;
		}
	}

	if (listener_ != -1)
		close(listener_);
}

Server::Server(const Server &other) : port_(other.port_), listener_(other.listener_), password_(other.password_) {}

Server &Server::operator=(const Server &other)
{
	if (this != &other)
	{
		port_ = other.port_;
		listener_ = other.listener_;
		password_ = other.password_;
	}

	return *this;
}

Server::Server(const char *port, const char *pass) : port_(port), listener_(-1), password_(pass)
{
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
}

void Server::run()
{
	std::cout << "[ircserver]: Waiting for connections" << std::endl;
	while (Server::signal_received_ == false)
	{
		int pool_count = poll(&connections_[0], connections_.size(), -1);

		if (pool_count == -1)
		{
			if (errno == EINTR)
				continue;

			throw std::runtime_error("poll failed " + std::string(std::strerror(errno)));
		}
		for (int i = connections_.size() - 1; i >= 0; i--)
		{
			if (connections_[i].revents & (POLLIN | POLLHUP))
			{
				if (connections_[i].fd == listener_)
				{
					acceptNewClient();
				}
				else
				{
					receiveClientData(i);
				}
			}
			// else if (connections_[i].revents & POLLOUT)
			// {
			// 	if (!sendClientData(i)) {
			// 		// TODO: Mejorar mensaje de error
			// 		std::cerr << "Error: Not all client<" << connections_[i].fd << ", " << clients_[connections_[i].fd].getFd() << "> data could be sent" << std::endl;
			// 	}
			// }
		}
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
		throw std::runtime_error("accept failed " + std::string(std::strerror(errno)));
	}

	fcntl(new_fd, F_SETFL, O_NONBLOCK);

	struct pollfd new_connection;

	new_connection.fd = new_fd;
	new_connection.events = POLLIN | POLLOUT;
	new_connection.revents = 0;

	connections_.push_back(new_connection);
	std::string remoteIp = getIpStr(reinterpret_cast<struct sockaddr *>(&remoteaddr));

	// TODO: Create Client object and add it to the clients_ map
	clients_[new_fd] = Client(new_fd, remoteIp);

	std::cout << "[ircserver]: New connection from " << remoteIp << " on socket " << new_fd << std::endl;
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
			std::cout << "[ircserver]: Client " << client_fd << " disconnected" << std::endl;
		}
		else
		{
			std::cerr << "[ircserver]: recv failed on client " << client_fd << " " << std::strerror(errno) << std::endl;
		}

		disconnectClient(client_index);
		return;
	}

	std::cout << "[ircserver]: Received " << bytes_received << " bytes from client " << client_fd << std::endl;
	client.appendToReadBuf(buffer, bytes_received);

	// TODO: May be this will be deleted
	while (client.hasCompleteCommand())
	{
		std::string cmd = client.extractCommand();

		std::cout << "raw command: " << cmd << std::endl;
		Command command(cmd);

		std::cout << "Client <" << client.getFd() << ", " << client.getIp() << "> sent a command: " << std::endl;;

		command.printCommand();

	}
}

void Server::disconnectClient(size_t client_index) {
	int fd = connections_[client_index].fd;

	close(fd);
	connections_.erase(connections_.begin() + client_index);
	clients_.erase(fd);
}

bool Server::sendClientData(size_t client_index) {
	int fd = connections_[client_index].fd;
	Client &client = clients_[fd];

	const std::string &clientWriteBuf = client.getWriteBuf();

	if (clientWriteBuf.empty()) {
		return false;
	}

	ssize_t bytes_sent = send(fd, clientWriteBuf.c_str(), clientWriteBuf.size(), 0);

	if (bytes_sent > 0)
	{
		client.eraseFromWriteBuf(bytes_sent);
	} else if (bytes_sent == -1) {
		if (errno == EAGAIN || errno == EWOULDBLOCK)
		{
			return false;
		}

		return true;
	}
	return false;

}

// TODO: Separar a otro archivo
bool Server::signal_received_ = false;

void Server::signalHandler(int signal)
{
	(void)signal;
	Server::signal_received_ = true;
}
