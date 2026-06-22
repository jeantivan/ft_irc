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
	 * TODO: Aquí los errores básicos ya han sido chequeados.
	 * TODO: Ahora vamos tenemos que chequear si el destinatario es un canal o un usuario.
	 */
}
