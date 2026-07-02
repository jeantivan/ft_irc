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
	(void)client;
	(void)server;
}
