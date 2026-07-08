#include "Channel.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include <algorithm>

Channel::Channel() : name_(),
					 topic_(), members_(),
					 operators_(),
					 inviteOnly_(false),
					 topicRestricted_(true),
					 password_(),
					 userLimit_(0),
					 invitedUsers_() {}

Channel::Channel(const Channel &other) : name_(other.name_),
										 topic_(other.topic_),
										 members_(other.members_),
										 operators_(other.operators_),
										 inviteOnly_(other.inviteOnly_),
										 topicRestricted_(other.topicRestricted_),
										 password_(other.password_),
										 userLimit_(other.userLimit_),
										 invitedUsers_(other.invitedUsers_) {}

Channel::~Channel() {}

Channel &Channel::operator=(const Channel &other)
{
	if (this != &other)
	{
		name_ = other.name_;
		topic_ = other.topic_;
		members_ = other.members_;
		operators_ = other.operators_;
		inviteOnly_ = other.inviteOnly_;
		topicRestricted_ = other.topicRestricted_;
		password_ = other.password_;
		userLimit_ = other.userLimit_;
		invitedUsers_ = other.invitedUsers_;
	}

	return *this;
}

Channel::Channel(const std::string &name) : name_(name),
											topic_(),
											members_(),
											operators_(),
											inviteOnly_(false),
											topicRestricted_(true),
											password_(),
											userLimit_(0),
											invitedUsers_() {}

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

bool Channel::isInviteOnly() const
{
	return inviteOnly_;
}

bool Channel::isTopicRestricted() const
{
	return topicRestricted_;
}

const std::string &Channel::getPassword() const
{
	return password_;
}

unsigned int Channel::getUserLimit() const
{
	return userLimit_;
}

bool Channel::isInvited(int fd) const
{
	return invitedUsers_.find(fd) != invitedUsers_.end();
}

// Setters
void Channel::setTopic(const std::string &topic)
{
	topic_ = topic;
}

void Channel::setInviteOnly(bool state)
{
	inviteOnly_ = state;
}

void Channel::setTopicRestricted(bool state)
{
	topicRestricted_ = state;
}

void Channel::setPassword(const std::string &pass)
{
	password_ = pass;
}

void Channel::setUserLimit(unsigned int limit)
{
	userLimit_ = limit;
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

void Channel::addInvited(int fd)
{
	invitedUsers_.insert(fd);
}

void Channel::removeInvited(int fd)
{
	invitedUsers_.erase(fd);
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
		if (it != members_.begin())
			nickList += " ";

		if (operators_.find(it->second->getFd()) != operators_.end())
			nickList += "@";
		else
			nickList += "+";

		nickList += it->second->getNick();
	}
	std::cout << "[DEBUGG] ln 124" << nickList << std::endl; // BORRAESTO
	return nickList;
}
