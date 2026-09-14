#include "Server.hpp"
#include "ResponseBuilder.hpp"
#include "Command/TopicCommand.hpp"

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

bool Server::isAchannel(const std::string &channel) const
{
	if (_channels_.find(channel) == _channels_.end())
		return false;
	else
		return true;
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
