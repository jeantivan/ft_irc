#include "Command/UnknownCommand.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "ResponseBuilder.hpp"
#include "Channel.hpp"

UnknownCommand::UnknownCommand() : Command("UNKNOWN", std::vector<std::string>()) {}

UnknownCommand::UnknownCommand(const UnknownCommand &other) : Command(other) {}

UnknownCommand::~UnknownCommand() {}

UnknownCommand &UnknownCommand::operator=(const UnknownCommand &other)
{
	if (this != &other)
	{
		Command::operator=(other);
	}

	return *this;
}

UnknownCommand::UnknownCommand(const std::string &type, const std::vector<std::string> &params) : Command(type, params) {}

Command *UnknownCommand::create(const std::string &type, const std::vector<std::string> &params)
{
	return new UnknownCommand(type, params);
}

// TODO: Tal vez no lo necesitemos
void UnknownCommand::execute(Client *client, Server *server)
{
	ResponseBuilder response;

	response.prefix(server->getName())
		.numeric(ERR_UNKNOWNCOMMAND)
		.target(client->getNick().empty() ? "*" : client->getNick())
		.params(type_)
		.trailing("Unknown command");

	server->queueClientData(*client, response.build());
	std::cout << "[ircserv]: ERR_UNKNOWNCOMMAND " << response.build() << std::endl;
}
