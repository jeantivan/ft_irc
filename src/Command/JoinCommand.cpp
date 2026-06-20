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

void JoinCommand::execute(Client *client, Server *server)
{
	ResponseBuilder response;
	std::vector<std::string> chanNames;
	std::vector<std::string> channPasword;

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
		//TO DO:
		// enviar NEDMOREPARAMS
		response.prefix(server->getName())
			.numeric(ERR_NEEDMOREPARAMS)
			.target(client->getNick())
			.trailing("Not enough parameters");
		server->queueClientData(*client, response.build());
		std::cerr << "[ircserver]: Error: ERR_NEEDMOREPARAMS" << std::endl;
		return;
	}
	chanNames = splitByComma(params_[0]);

	if (params_.size() >= 2)
		channPasword = splitByComma(params_[1]);
	
	for(size_t i = 0; i < chanNames.size(); i++)
	{
		Channel *channel_i;
		if (!parseChannName(chanNames[i]))
		{
			response.prefix(server->getName())
				.numeric(ERR_BADCHANNAME)
				.target(client->getNick())
				.trailing("Illegal channel name");
			std::cerr << "[ircserver]: Error: ERR_BADCHANNAME" << std::endl;
			return;
		}
		if (i < channPasword.size())
			server->joinChannel(client, chanNames[i], channPasword[i]);
		else
			server->joinChannel(client, chanNames[i], NULL);
	}
	return;
}

Command *JoinCommand::create(const std::string &type, const std::vector<std::string> &params)
{
	return new JoinCommand(type, params);
}

// función auxiliar a execute
static std::vector<std::string> splitByComma(const std::string& str) {
    std::vector<std::string> tokens;
    std::istringstream ss(str);
    std::string token;

    while (std::getline(ss, token, ',')) {
        tokens.push_back(token);
    }
    return tokens;
}

bool parseChannName(std::string name)
{
	if (name[0] != '#')
		return false;
	if (name.length() > 50 || name.length() < 2)
		return false;
	if (name.find_first_of(" \a:\r\n"))
		return false;
	return true;
}
