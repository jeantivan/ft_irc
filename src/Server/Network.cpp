#include "Server.hpp"
#include "Command/CommandFactory.hpp"

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
			continue;
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

size_t Server::findConnectionByFd(int fd) const
{
	for (size_t i = 0; i < connections_.size(); i++)
	{
		if (connections_[i].fd == fd)
			return i;
	}
	return static_cast<size_t>(-1);
}
