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
	(void)client;
	(void)server;
}
