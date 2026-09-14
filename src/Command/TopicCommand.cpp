#include "Command/TopicCommand.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "ResponseBuilder.hpp"

TopicCommand::TopicCommand() : Command("TOPIC", std::vector<std::string>()) {}

TopicCommand::TopicCommand(const TopicCommand &other) : Command(other) {}

TopicCommand &TopicCommand::operator=(const TopicCommand &other)
{
	if (this != &other)
	{
		Command::operator=(other);
	}

	return *this;
}

TopicCommand::~TopicCommand() {}

TopicCommand::TopicCommand(const std::string &type, const std::vector<std::string> &params) : Command(type, params) {}

void TopicCommand::execute(Client *client, Server *server)
{
	ResponseBuilder response;
	int clientFd = client->getFd();

	if (!client->isAuth())
	{
		// enviar ERR_NOTREGISTERED
		server->sendNumericReply(client, ERR_NOTREGISTERED, "", "You have not registered");
		std::cerr << "[ircserver]: Error: ERR_NOTREGISTERED" << std::endl;
		return;
	}

	if (params_.size() == 0)
	{
		server->sendNumericReply(client, ERR_NEEDMOREPARAMS, "TOPIC", "Not enough parameters");
		return;
	}

	Channel *channel = server->getChannel(params_[0]);
	if (channel == NULL)
	{
		server->sendNumericReply(client, ERR_NOSUCHCHANNEL, params_[0], "No such channel");
		return;
	}
	std::string channName = channel->getName();

	if (params_.size() == 1)
	{
		if(!channel->isMember(clientFd))
		{
			server->sendNumericReply(client, ERR_NOTONCHANNEL, channName, "You're not on that channel");
			return;
		}
		std::string topic = channel->getTopic();
		if (topic.empty())
		{
			server->sendNumericReply(client, RPL_NOTOPIC, channName, "No topic is set");
			return;
		}

		server->sendNumericReply(client, RPL_TOPIC, channName, topic);
		server->sendNumericReply(client, RPL_TOPICWHOTIME, channName + " " + channel->getTopicAuthor() + " " + channel->getTopicTime(), "");
		return;
	}
	if (params_.size() > 1)
	{
		if(!channel->isMember(clientFd))
		{
			server->sendNumericReply(client, ERR_NOTONCHANNEL, channName, "You're not on that channel");
			return;
		}

		if (channel->isTopicRestricted() && !channel->isOperator(clientFd))
		{
			server->sendNumericReply(client, ERR_CHANOPRIVSNEEDED, channName, "You're not channel operator");
			return;
		}

		channel->setTopic(params_[1], client->getNick());
		response.prefix(client->getPrefix()).command("TOPIC").target(channName).trailing(params_[1]);
		channel->broadcastAll(response.build(), server);
	}
	return;
}

Command *TopicCommand::create(const std::string &type, const std::vector<std::string> &params)
{
	return new TopicCommand(type, params);
}
