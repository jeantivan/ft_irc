#include "Command/ModeCommand.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "ResponseBuilder.hpp"
#include "NumericReplies.hpp"
#include "Mode/ModeHandler.hpp"

ModeCommand::ModeCommand() : Command("MODE", std::vector<std::string>()) {}

ModeCommand::ModeCommand(const ModeCommand &other) : Command(other) {}

ModeCommand &ModeCommand::operator=(const ModeCommand &other)
{
	if (this != &other)
	{
		Command::operator=(other);
	}

	return *this;
}

ModeCommand::~ModeCommand() {}

ModeCommand::ModeCommand(const std::string &type, const std::vector<std::string> &params) : Command(type, params) {}

Command *ModeCommand::create(const std::string &type, const std::vector<std::string> &params)
{
	return new ModeCommand(type, params);
}

void ModeCommand::handleChannelMode(Channel *channel, Client *client, Server *server) const
{
	ResponseBuilder response;
	std::string activesModes = "+";
	std::string modeArgs = "";

	if (channel->isInviteOnly())
		activesModes += "i";

	if (channel->isTopicRestricted())
		activesModes += "t";

	if (!channel->getPassword().empty())
	{
		activesModes += "k";
		modeArgs += " " + channel->getPassword();
	}

	if (channel->getUserLimit() > 0)
	{
		activesModes += "l";

		std::stringstream ss;
		ss << channel->getUserLimit();
		modeArgs += " " + ss.str();
	}

	if (activesModes == "+")
		activesModes = "";

	response.prefix(server->getName())
		.numeric(RPL_CHANNELMODEIS)
		.target(client->getNick())
		.params(channel->getName() + " " + activesModes + modeArgs);

	server->queueClientData(*client, response.build());
	std::cout << "[ircserver]: Replied with RPL_CHANNELMODEIS for " << channel->getName() << std::endl;
}

void ModeCommand::applyChanges(Channel *channel, Client *client, Server *server) const
{
	ResponseBuilder response;

	// Parse Modes
	std::string modes = params_[1];
	bool isAdding = true;
	size_t paramIndex = 2;

	// Save changes applies
	std::string appliedModes = "";
	std::string appliedArgs = "";
	char lastSign = '\0';

	for (size_t i = 0; i < modes.length(); ++i)
	{
		char c = modes[i];

		if (c == '+')
		{
			isAdding = true;
			continue;
		}
		else if (c == '-')
		{
			isAdding = false;
			continue;
		}

		ModeHandler *handler = server->getModeHandler(c);

		if (!handler)
		{
			response
				.prefix(server->getName())
				.numeric(ERR_UNKNOWNCOMMAND)
				.target(client->getNick())
				.params(channel->getName())
				.trailing("Unknown mode comand");
			server->queueClientData(*client, response.build());
			std::cout << "[ircserver]: Error ERR_UNKNOWNCOMMAND " << response.build() << std::endl;
			return;
		}

		std::string modeParam = "";

		if (handler->requiresParam(isAdding))
		{
			if (paramIndex < params_.size())
			{
				modeParam = params_[paramIndex];
				paramIndex++;
			}
			else
			{
				continue;
			}
		}

		bool changed = handler->change(channel, isAdding, modeParam, client, server);
		if (changed)
		{
			char currSign = isAdding ? '+' : '-';

			if (lastSign != currSign)
			{
				appliedModes += currSign;
				lastSign = currSign;
			}

			appliedModes += c;

			if (!modeParam.empty())
			{
				if (!appliedArgs.empty())
				{
					appliedArgs += " ";
				}
				appliedArgs += modeParam;
			}
		}
	}

	if (!appliedModes.empty())
	{
		response.prefix(client->getPrefix())
			.command("MODE")
			.target(channel->getName())
			.params(appliedModes);

		if (!appliedArgs.empty())
		{
			response.params(appliedArgs);
		}

		std::cout << "[ircserver]: Mode Command response " << response.build() << std::endl;
		channel->broadcastAll(response.build(), server);
	}
}

void ModeCommand::execute(Client *client, Server *server)
{
	ResponseBuilder response;

	if (!client->isAuth())
	{
		response
			.prefix(server->getName())
			.numeric(ERR_NOTREGISTERED)
			.target(client->getNick().empty() ? "*" : client->getNick())
			.trailing("You have not registered");
		server->queueClientData(*client, response.build());
		std::cout << "[ircserver]: Error ERR_NOTREGISTERED " << type_ << std::endl;
		return;
	}

	std::string target = params_[0];

	if (target[0] != '#')
		return;

	Channel *channel = server->getChannel(params_[0]);
	if (!channel)
	{
		response.prefix(server->getName())
			.numeric(ERR_NOSUCHCHANNEL)
			.target(client->getNick())
			.params(params_[0])
			.trailing("No such channel");
		server->queueClientData(*client, response.build());
		std::cout << "[ircserver]: Error ERR_NOSUCHCHANNEL " << response.build() << std::endl;
		return;
	}

	if (!channel->isMember(client->getFd()))
	{
		response.prefix(server->getName())
			.numeric(ERR_NOTONCHANNEL)
			.target(client->getNick())
			.params(params_[0])
			.trailing("You are not member of this channel");
		server->queueClientData(*client, response.build());
		std::cout << "[ircserver]: Error ERR_NOTONCHANNEL " << response.build() << std::endl;
		return;
	}

	// TODO: Send RPL_CHANNELMODEIS when "MODE #channel"
	if (params_.size() == 1)
	{
		handleChannelMode(channel, client, server);
		return;
	}

	if (!channel->isOperator(client->getFd()))
	{
		response
			.prefix(server->getName())
			.numeric(ERR_CHANOPRIVSNEEDED)
			.target(client->getNick())
			.params(channel->getName())
			.trailing("You're not channel operator");
		server->queueClientData(*client, response.build());
		std::cout << "[ircserver]: Error ERR_CHANOPRIVSNEEDED " << response.build() << std::endl;
		return;
	}

	applyChanges(channel, client, server);
}
