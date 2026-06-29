#include "Command/PrivMsgCommand.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "ResponseBuilder.hpp"
#include "Channel.hpp"

PrivMsgCommand::PrivMsgCommand() : Command("PRIVMSG", std::vector<std::string>()) {}

PrivMsgCommand::PrivMsgCommand(const PrivMsgCommand &other) : Command(other) {}

PrivMsgCommand::~PrivMsgCommand() {}

PrivMsgCommand &PrivMsgCommand::operator=(const PrivMsgCommand &other)
{
	if (this != &other)
	{
		Command::operator=(other);
	}

	return *this;
}

PrivMsgCommand::PrivMsgCommand(const std::string &type, const std::vector<std::string> &params) : Command(type, params) {}

Command *PrivMsgCommand::create(const std::string &type, const std::vector<std::string> &params)
{
	return new PrivMsgCommand(type, params);
}

std::string PrivMsgCommand::checkParams(Client *client, Server *server) const
{
	ResponseBuilder response;
	response.prefix(server->getName());
	bool hasError = true;

	if (!client->isAuth())
	{
		response.numeric(ERR_NOTREGISTERED)
			.target(client->getNick().empty() ? "*" : client->getNick())
			.trailing("You have not registered");
		std::cout << "[ircserver]: Error ERR_NOTREGISTERED" << std::endl;
	}
	else if (params_.size() < 1 || params_[0].empty())
	{
		response.numeric(ERR_NORECIPIENT)
			.target(client->getNick())
			.trailing("no recipient given (" + type_ + ")");
		std::cout << "[ircserver]: Error ERR_NORECIPIENT" << std::endl;
	}
	else if (params_.size() < 2 || params_[1].empty())
	{
		response.numeric(ERR_NOTEXTTOSEND)
			.target(client->getNick())
			.trailing("No text to send");
		std::cout << "[ircserver]: Error ERR_NOTEXTTOSEND" << std::endl;
	}
	else
	{
		hasError = false;
	}

	if (hasError)
		return response.build();

	return "";
}

void PrivMsgCommand::handleUserResponse(Client *client, Server *server, const std::string &target) const
{
	ResponseBuilder response;

	Client *recipient = server->findClientByNick(target);

	if (!recipient)
	{
		response.prefix(server->getName())
			.numeric(ERR_NOSUCHNICK)
			.target(client->getNick())
			.params(target)
			.trailing("No such nick");
		server->queueClientData(*client, response.build());
		std::cout << "[ircserver]: Error ERR_NOSUCHNICK" << params_[0] << std::endl;

		return;
	}

	response.prefix(client->getPrefix())
		.command("PRIVMSG")
		.target(recipient->getNick())
		.trailing(params_[1]);
	server->queueClientData(*recipient, response.build());
	std::cout << "[ircserver]: Good: " << response.build() << std::endl;
}

void PrivMsgCommand::handleChannelResponse(Client *client, Server *server, const std::string &target) const
{
	ResponseBuilder response;

	Channel *channel = server->getChannel(target);

	if (!channel)
	{
		response.prefix(server->getName())
			.numeric(ERR_NOSUCHCHANNEL)
			.target(client->getNick())
			.params(target)
			.trailing("No such channel");
		server->queueClientData(*client, response.build());
		std::cout << "[ircserver]: Error ERR_NOSUCHCHANNEL" << std::endl;
		return;
	}

	if (!channel->isMember(client->getFd()))
	{
		response.prefix(server->getName())
			.numeric(ERR_NOTONCHANNEL)
			.target(client->getNick())
			.params(target)
			.trailing("You're not on that channel");
		server->queueClientData(*client, response.build());
		std::cout << "[ircserver]: ERR_NOTONCHANNEL" << std::endl;
		return;
	}

	response.prefix(client->getPrefix())
		.command("PRIVMSG")
		.target(channel->getName())
		.trailing(params_[1]);
	std::cout << "[ircserver]: Good " << response.build() << std::endl;

	channel->broadcast(response.build(), client->getFd(), server);
}

void PrivMsgCommand::execute(Client *client, Server *server)
{
	std::string errorParams = checkParams(client, server);

	if (!errorParams.empty())
	{
		server->queueClientData(*client, errorParams);
		return;
	}

	std::vector<std::string> targets = splitByComma(params_[0]);

	for (size_t i = 0; i < targets.size(); ++i)
	{
		std::string currentTarget = targets[i];

		if (currentTarget.empty())
			continue;

		if (currentTarget[0] == '#')
		{
			handleChannelResponse(client, server, currentTarget);
		}
		else
		{
			handleUserResponse(client, server, currentTarget);
		}
	}
}
