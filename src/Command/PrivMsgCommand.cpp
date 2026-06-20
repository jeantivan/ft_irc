#include "Command/PrivMsgCommand.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "ResponseBuilder.hpp"

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

void PrivMsgCommand::execute(Client *client, Server *server)
{
}
