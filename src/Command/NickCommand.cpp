#include "Command/NickCommand.hpp"

NickCommand::NickCommand() : Command("NICK", std::vector<std::string>()) {}

NickCommand::NickCommand(const NickCommand &other) : Command(other) {}

NickCommand::~NickCommand() {}

NickCommand &NickCommand::operator=(const NickCommand &other)
{
	if (this != &other)
	{
		Command::operator=(other);
	}

	return *this;
}

NickCommand::NickCommand(const std::string &type, const std::vector<std::string> &params) : Command(type, params) {}

void NickCommand::execute(Client *client, Server *server)
{
}

// Creator
Command *NickCommand::create(const std::string &type, const std::vector<std::string> &params)
{
	return new NickCommand(type, params);
}
