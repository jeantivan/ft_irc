#include "Command/PassCommand.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "ResponseBuilder.hpp"

PassCommand::PassCommand() : Command("PASS", std::vector<std::string>()) {}

PassCommand::PassCommand(const PassCommand &other) : Command(other) {}

PassCommand &PassCommand::operator=(const PassCommand &other)
{
	if (this != &other)
	{
		Command::operator=(other);
	}

	return *this;
}

PassCommand::~PassCommand() {}

PassCommand::PassCommand(const std::string &type, const std::vector<std::string> &params) : Command(type, params) {}

void PassCommand::execute(Client *client, Server *server)
{
	ResponseBuilder response;
	if (params_.empty())
	{
		response.prefix(server->getName()).numeric(ERR_NEEDMOREPARAMS).target("*").params("PASS").trailing("Not enough parameters");
		std::cerr << "[ircserver]: Error: Bad command params." << std::endl;
	}
	else if (client->isAuth() == true)
	{
		response.prefix(server->getName()).numeric(ERR_ALREADYREGISTRED).target(client->getNick()).trailing("Unauthorized command (already registered)");
		std::cerr << "[ircserver]: Client <" << client->getFd() << " Error: Unauthorized command (already registered)" << std::endl;
	}
	else
	{
		client->setPassword(params_[0]);
		client->setAuthState(AUTH_PASS);
		return;
	}
	server->queueClientData(*client, response.build());
	return;
}

Command *PassCommand::create(const std::string &type, const std::vector<std::string> &params)
{
	return new PassCommand(type, params);
}
