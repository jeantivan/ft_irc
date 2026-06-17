#include "Command/CapCommand.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "ResponseBuilder.hpp"

CapCommand::CapCommand() : Command("CAP", std::vector<std::string>()) {}

CapCommand::CapCommand(const CapCommand &other) : Command(other) {}

CapCommand::~CapCommand() {}

CapCommand &CapCommand::operator=(const CapCommand &other)
{
	if (this != &other)
	{
		Command::operator=(other);
	}
	return *this;
}

CapCommand::CapCommand(const std::string &type, const std::vector<std::string> &params) : Command(type, params) {}

Command *CapCommand::create(const std::string &type, const std::vector<std::string> &params)
{
	return new CapCommand(type, params);
}

void CapCommand::execute(Client *client, Server *server)
{
	if (params_.empty())
		return;

	std::string subcommand = params_[0];

	std::string target = client->getNick().empty() ? "*" : client->getNick();

	if (subcommand == "LS")
	{
		std::string response = ":" + server->getName() + " CAP " + target + " LS :\r\n";

		server->queueClientData(*client, response);

		std::cout << "[ircserver]: Client <" << client->getFd()
				  << "> sent CAP LS. Responded with empty capabilities." << std::endl;
	}
	else if (subcommand == "REQ")
	{
		std::string requested_caps = (params_.size() > 1) ? params_[1] : "";
		std::string response = ":" + server->getName() + " CAP " + target + " NAK :" + requested_caps + "\r\n";

		server->queueClientData(*client, response);
	}
	else if (subcommand == "END")
	{
		std::cout << "[ircserver]: Client <" << client->getFd() << "> ended CAP negotiation." << std::endl;
	}
}
