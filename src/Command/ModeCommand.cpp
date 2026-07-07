#include "Command/ModeCommand.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "ResponseBuilder.hpp"
#include "NumericReplies.hpp"

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

	if (chan->isOperator(client->getFd()))
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
	//(void)client;
	//(void)server;
}
