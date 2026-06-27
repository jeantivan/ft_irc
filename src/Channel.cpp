#include "Channel.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include <algorithm>

Channel::Channel() : name_(), topic_(), members_(), operators_() {}

Channel::Channel(const Channel &other) : name_(other.name_), topic_(other.topic_), members_(other.members_), operators_(other.operators_) {}

Channel::~Channel() {}

Channel &Channel::operator=(const Channel &other)
{
	if (this != &other)
	{
		name_ = other.name_;
		topic_ = other.topic_;
		members_ = other.members_;
		operators_ = other.operators_;
	}

	return *this;
}

Channel::Channel(const std::string &name) : name_(name), topic_(), members_(), operators_() {}

// Getters
const std::string &Channel::getName() const
{
	return name_;
}

const std::string &Channel::getTopic() const
{
	return topic_;
}

const std::map<int, Client *> &Channel::getMembers() const
{
	return members_;
}

// Setters
void Channel::setTopic(const std::string &topic)
{
	topic_ = topic;
}

// Members
void Channel::addClient(Client *client)
{
	members_[client->getFd()] = client;
}

void Channel::removeClient(int fd)
{
	std::map<int, Client *>::iterator it = members_.find(fd);

	if (it != members_.end())
	{
		members_.erase(it);
	}

	if (isOperator(fd))
	{
		removeOperator(fd);
	}
}

bool Channel::isMember(int fd) const
{
	std::map<int, Client *>::const_iterator it = members_.find(fd);
	return it != members_.end();
}

bool Channel::isOperator(int fd) const
{
	return operators_.find(fd) != operators_.end();
}

void Channel::addOperator(int fd)
{
	operators_.insert(fd);
}

void Channel::removeOperator(int fd)
{
	operators_.erase(fd);
}

bool Channel::isEmpty() const
{
	return members_.empty();
}

// Messages
void Channel::broadcast(const std::string &message, int sender_fd, Server *server) const
{
	for (std::map<int, Client *>::const_iterator it = members_.begin(); it != members_.end(); ++it)
	{
		if (it->first == sender_fd)
			continue;
		server->queueClientData(*(it->second), message);
	}
}

void Channel::broadcastAll(const std::string &message, Server *server) const
{
	for (std::map<int, Client *>::const_iterator it = members_.begin(); it != members_.end(); ++it)
	{
		server->queueClientData(*(it->second), message);
	}
}

std::string Channel::getNickList() const
{
	std::map<int, Client *>::const_iterator it;
	std::string nickList;

	for (it = members_.begin(); it != members_.end(); it++)
	{
		if(it != members_.begin())
			nickList += " ";

		if(operators_.find(it->second->getFd()) != operators_.end())
			nickList += "@";
		else
			nickList += "+";

		nickList += it->second->getNick();
	}
std::cout << "[DEBUGG] ln 124" << nickList << std::endl; //BORRAESTO
	return nickList;
}
