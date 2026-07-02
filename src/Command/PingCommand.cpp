#include "Command/PingCommand.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "ResponseBuilder.hpp"

PingCommand::PingCommand() : Command("PING", std::vector<std::string>()) {}

PingCommand::PingCommand(const PingCommand &other) : Command(other) {}

PingCommand::~PingCommand() {}

PingCommand &PingCommand::operator=(const PingCommand &other)
{
	if (this != &other)
	{
		Command::operator=(other);
	}

	return *this;
}

PingCommand::PingCommand(const std::string &type, const std::vector<std::string> &params) : Command(type, params) {}

Command *PingCommand::create(const std::string &type, const std::vector<std::string> &params)
{
	return new PingCommand(type, params);
}

void PingCommand::execute(Client *client, Server *server)
{
	ResponseBuilder response;

	if (params_.empty() || (params_.size() < 2 && params_[0].empty()))
	{
		response.prefix(server->getName())
			.numeric(ERR_NOORIGIN)
			.target(client->getNick().empty() ? "*" : client->getNick())
			.trailing("No origin specified");
		server->queueClientData(*client, response.build());
		std::cout << "[ircserv]: ERR_NOORIGIN " << response.build() << std::endl;
		return;
	}

	if (params_.size() > 1 && params_[1] != server->getName())
	{
		response.prefix(server->getName())
			.numeric(ERR_NOSUCHSERVER)
			.target(client->getNick().empty() ? "*" : client->getNick())
			.params(params_[1])
			.trailing("No such server");
		server->queueClientData(*client, response.build());
		std::cout << "[ircserv]: ERR_NOSUCHSERVER " << response.build() << std::endl;
		return;
	}

	response.prefix(server->getName())
		.target(server->getName())
		.command("PONG")
		.trailing(params_[0]);
	server->queueClientData(*client, response.build());
	std::cout << "[ircserv]: PONG response " << response.build() << std::endl;
}
