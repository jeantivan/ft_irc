#include "Command/PongCommand.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "ResponseBuilder.hpp"

PongCommand::PongCommand() : Command("PONG", std::vector<std::string>()) {}

PongCommand::PongCommand(const PongCommand &other) : Command(other) {}

PongCommand::~PongCommand() {}

PongCommand &PongCommand::operator=(const PongCommand &other)
{
	if (this != &other)
	{
		Command::operator=(other);
	}

	return *this;
}

PongCommand::PongCommand(const std::string &type, const std::vector<std::string> &params) : Command(type, params) {}

Command *PongCommand::create(const std::string &type, const std::vector<std::string> &params)
{
	return new PongCommand(type, params);
}

void PongCommand::execute(Client *client, Server *server)
{

	if (params_.empty() || params_[0].empty() || params_[0].size() > 19) // enviamos un time_t como token, no puede tener mas de 19 digitos
	{
		std::cout << "[ircserver]: " << client->getNick() << " ha enviado un PONG mal formado" << std::endl;
		return;
	}
	else
	{
		long long tokenPong = std::atoll(params_[0].c_str());
		if(tokenPong == 0)
		{
			std::cout << "[ircserver]: " << client->getNick() << " ha enviado un PONG mal formado" << std::endl;
			return;
		}
		client->setLastPong(static_cast<time_t>(tokenPong));
		std::cout << "[ircserver]: " << client->getNick() << " send PONG " << params_[0] << std::endl;
	}

}
