#include "Command/PrivMsgCommand.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "ResponseBuilder.hpp"

PrivMsgCommand::PrivMsgCommand() : Command("PRIVMSG", std::vector<std::string>()) {}

PrivMsgCommand::PrivMsgCommand(const PrivMsgCommand &other) : Command(other) {}

PrivMsgCommand::~PrivMsgCommand() {}

PrivMsgCommand &PrivMsgCommand::operator=(const PrivMsgCommand &other)
{
	if (this != &other)
	{
		Command::operator=(other);
	}

	return *this;
}

PrivMsgCommand::PrivMsgCommand(const std::string &type, const std::vector<std::string> &params) : Command(type, params) {}

Command *PrivMsgCommand::create(const std::string &type, const std::vector<std::string> &params)
{
	return new PrivMsgCommand(type, params);
}

void PrivMsgCommand::execute(Client *client, Server *server)
{
	ResponseBuilder response;

	response.prefix(server->getName());

	// Client not authenticated
	if (!client->isAuth())
	{
		response.numeric(ERR_NOTREGISTERED)
			.target(client->getNick().empty() ? "*" : client->getNick())
			.trailing("You have not registered");
		server->queueClientData(*client, response.build());
		return;
	}

	// Bad params sent from client.
	if (params_.size() != 2)
	{
		response.numeric(ERR_NEEDMOREPARAMS)
			.target(client->getNick())
			.trailing(type_ + ": only need two params");
		server->queueClientData(*client, response.build());
		return;
	}

	// Not recipient
	if (params_[0].empty())
	{
		response.numeric(ERR_NORECIPIENT)
			.target(client->getNick())
			.trailing("no recipient given " + type_);
		server->queueClientData(*client, response.build());
		return;
	}

	// Not message to send
	if (params_[0].empty())
	{
		response.numeric(ERR_NOTEXTTOSEND)
			.target(client->getNick())
			.trailing("no text to send");
		server->queueClientData(*client, response.build());
		return;
	}

	/**
	 * TODO: Ahora vamos tenemos que chequear si el destinatario es un canal o un usuario.
	 */

	// Enviamos el mensaje a un usuario
	Client *recipient = server->findClientByNick(params_[0]);

	if (!recipient)
	{
		response.numeric(ERR_NOSUCHNICK)
			.target(client->getNick())
			.trailing(params_[0] + ": No such nick/channel");
		server->queueClientData(*client, response.build());
		return;
	}

	response.clear();
	response.prefix(client->getPrefix())
		.command("PRIVMSG")
		.target(recipient->getNick())
		.trailing(params_[1]);
	server->queueClientData(*recipient, response.build());
}
