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
		response.prefix(server->getName()).numeric(ERR_NEEDMOREPARAMS).target(client->getNick()).params("PASS").trailing("Not enough parameters");
		std::cerr << "[ircserver]: Error: Bad command params." << std::endl;
	}
	else if (client->isAuth())
	{
		
		response.prefix(server->getName()).numeric(ERR_ALREADYREGISTRED).target(client->getNick()).trailing("Unauthorized command (already registered)");
	}
	else if (params_[0] != server->getPassword())
	{

		response.prefix(server->getName()).numeric(ERR_PASSWDMISMATCH).target(client->getNick()).trailing("Password incorrect");
		server->disconnectClient(client->getFd());
	}
	else
	{
		client->setAuthState(AUTH_PASS);
		std::cout << "[ircserver]: Client <" << client->getFd() << ", " << client->getIp() << "> is authenticated" << std::endl;
	}
	// TODO: Should I write something to respond when auth is success


	client->appendToWriteBuf(response.build());
	return;
}

Command *PassCommand::create(const std::string &type, const std::vector<std::string> &params)
{
	return new PassCommand(type, params);
}
