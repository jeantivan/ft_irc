#include "Command/PassCommand.hpp"
#include "Server.hpp"
#include "Client.hpp"

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

	if (params_.size() != 1)
	{
		// TODO: Handle bad command params error, this should be wrote on the client's writeBuf_;
		

		std::cerr << "[ircserver]: Error: Bad command params." << std::endl;
		return;
	}

	if (client->isAuth())
	{
		// TODO: Should I sent a message that client is already auth?
		return;
	}

	if (params_[0] != server->getPassword())
	{
		server->disconnectClient(client->getFd());
		return;
	}

	client->setAuth(true);
	std::cout << "[ircserver]: Client <" << client->getFd() << ", " << client->getIp() << "> is authenticated" << std::endl;

	// TODO: Should I write something to respond when auth is success
	return;
}

Command *PassCommand::create(const std::string &type, const std::vector<std::string> &params)
{
	return new PassCommand(type, params);
}
