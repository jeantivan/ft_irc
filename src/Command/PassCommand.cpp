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
	else if (client->getState() & AUTH_PASS)
	{
		
		response.prefix(server->getName()).numeric(ERR_ALREADYREGISTRED).target("*").trailing("Unauthorized command (already registered)");
		std::cerr << "[ircserver]: Client <" << client->getFd() << " Error: Unauthorized command (already registered)" << std::endl;
	}
	else if (params_[0] != server->getPassword())
	{

		response.prefix(server->getName()).numeric(ERR_PASSWDMISMATCH).target("*").trailing("Password incorrect");
		std::cerr<< "[ircserver]--->" << client->getFd() << " Error: Password incorrect" << std::endl;		
		//server->disconnectClient(client->getFd());
		client->setToDisconnect();
	}
	else
	{
		client->setAuthState(AUTH_PASS);
		std::cout << "[ircserver]: Client <" << client->getFd() << ", " << client->getIp() << "> is authenticated" << std::endl;
		return;	
	}

	server->queueClientData(*client, response.build());
	return;
}

Command *PassCommand::create(const std::string &type, const std::vector<std::string> &params)
{
	return new PassCommand(type, params);
}
