#include "Command/UserCommand.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "ResponseBuilder.hpp"

UserCommand::UserCommand() : Command("USER", std::vector<std::string>()) {}

UserCommand::UserCommand(const UserCommand &other) : Command(other) {}

UserCommand &UserCommand::operator=(const UserCommand &other)
{
	if (this != &other)
	{
		Command::operator=(other);
	}

	return *this;
}

UserCommand::~UserCommand() {}

UserCommand::UserCommand(const std::string &type, const std::vector<std::string> &params) : Command(type, params) {}

void UserCommand::execute(Client *client, Server *server)
{
	ResponseBuilder response; 
	if (client->isAuth() == true)
	{
		response.prefix(server->getName()).numeric(ERR_ALREADYREGISTRED).target("*").trailing("Unauthorized command (already registered)");
		std::cerr << "[ircserver]: Client <" << client->getFd() << " Error: Unauthorized command (already registered)" << std::endl;
		server->queueClientData(*client, response.build());
		return;
	}
	else if(params_.size() < 4)
	{
		response.prefix(server->getName()).numeric(ERR_NEEDMOREPARAMS).target("*").params("PASS").trailing("Not enough parameters");
		std::cerr << "[ircserver]: Error: Bad command params." << std::endl;
		server->queueClientData(*client, response.build());
		return;
	}
	else
	{
		client->setAuthState(AUTH_USER);
		ResponseBuilder response;
		client->setUser(params_[0]);
		//param[1] es mode, y no lo implementa irc, se ignora
		//param[2] siempre es "*" en desuso
		client->setRealname(params_[3]);
		server->requestRegistration(*client);
	}
}

Command *UserCommand::create(const std::string &type, const std::vector<std::string> &params)
{
	return new UserCommand(type, params);
}
