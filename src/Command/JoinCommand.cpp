#include "Command/JoinCommand.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "ResponseBuilder.hpp"
#include "NumericReplies.hpp"

JoinCommand::JoinCommand() : Command("JOIN", std::vector<std::string>()) {}

JoinCommand::JoinCommand(const JoinCommand &other) : Command(other) {}

JoinCommand &JoinCommand::operator=(const JoinCommand &other)
{
	if (this != &other)
	{
		Command::operator=(other);
	}

	return *this;
}

JoinCommand::~JoinCommand() {}

JoinCommand::JoinCommand(const std::string &type, const std::vector<std::string> &params) : Command(type, params) {}

// Funciones auxiliares a execute()
static bool parseChannName(const std::string name)
{
    if (name.empty())
		return false;
	if (name[0] != '#')
		return false;
	if (name.length() > 50 || name.length() < 2)
		return false;
	if (name.find_first_of(" \a:\r\n") != std::string::npos)
		return false;
	return true;
}

void JoinCommand::execute(Client *client, Server *server)
{
	ResponseBuilder response;

	if (!client->isAuth())
	{
		// enviar ERR_NOTREGISTERED
		response.prefix(server->getName())
			.numeric(ERR_NOTREGISTERED)
			.target(client->getNick())
			.trailing("You have not registered");
		server->queueClientData(*client, response.build());
		std::cerr << "[ircserver]: Error: ERR_NOTREGISTERED" << std::endl;
		return;
	}

	if (params_.size() < 1)
	{
		response.prefix(server->getName())
			.numeric(ERR_NEEDMOREPARAMS)
			.target(client->getNick())
			.trailing("Not enough parameters");
		server->queueClientData(*client, response.build());
		std::cerr << "[ircserver]: Error: ERR_NEEDMOREPARAMS" << std::endl;
		return;
	}



	std::vector<std::string> chanNames = splitByComma(params_[0]);
	std::vector<std::string> channPasword;

	if (params_.size() >= 2)
		channPasword = splitByComma(params_[1]);

	for (size_t i = 0; i < chanNames.size(); i++)
	{

		if (chanNames[i] == "0") // el nombre chanNames[] queda mal aqui, tcnicamente 0 no es un nombre de canal
		{
			std::vector<std::string> channelsToLeave;
			std::map<std::string, Channel> &channels = server->getChannels();
			int clientFd = client->getFd();

			for (std::map<std::string, Channel>::iterator it = channels.begin(); it != channels.end(); ++it)
			{
				if (it->second.isMember(clientFd))
					channelsToLeave.push_back(it->first);
			}
			for (size_t i = 0; i < channelsToLeave.size(); ++i)
				server->leaveChannel(client, channelsToLeave[i], "Left all channels");
			continue;
		}

		if (!parseChannName(chanNames[i]))
		{
			response.prefix(server->getName())
				.numeric(ERR_BADCHANNAME)
				.target(client->getNick())
				.trailing("Illegal channel name");
			std::cerr << "[ircserver]: Error: ERR_BADCHANNAME" << std::endl;
			server->queueClientData(*client, response.build());
			continue; // OJOOO no descartar este cambio SI es importante, queremos continuar parseando nombres aunque uno de la lista este mal formado.
		}
		if (i < channPasword.size())
			server->joinChannel(client, chanNames[i], channPasword[i]);
		else
			server->joinChannel(client, chanNames[i], "");
	}
	return;
}

Command *JoinCommand::create(const std::string &type, const std::vector<std::string> &params)
{
	return new JoinCommand(type, params);
}
