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

	// Do nothing if target is and User/client
	// if (target[0] != '#')
	// 	return;

	// TODO: Send RPL_CHANNELMODEIS when "MODE #channel"
	if (params_.size() == 1)
	{
		std::cout << "[ircserver]: MODE " << target << " received" << std::endl;
		return;
	}

	Channel *chan = server->getChannel(params_[0]);
	if (!chan)
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

	if (!chan->isOperator(client->getFd()))
	{
		response
			.prefix(server->getName())
			.numeric(ERR_CHANOPRIVSNEEDED)
			.target(client->getNick())
			.params(chan->getName())
			.trailing("You're not channel operator");
		server->queueClientData(*client, response.build());
		std::cout << "[ircserver]: Error ERR_CHANOPRIVSNEEDED " << response.build() << std::endl;
		return;
	}

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
				.params(chan->getName())
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

		bool changed = handler->change(chan, isAdding, modeParam);
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
			.target(chan->getName())
			.params(appliedModes);

		if (!appliedArgs.empty())
		{
			response.params(appliedArgs);
		}

		std::cout << "[ircserver]: Mode Command response " << response.build() << std::endl;
		chan->broadcastAll(response.build(), server);
	}
}
