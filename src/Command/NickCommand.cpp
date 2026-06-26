#include "Command/NickCommand.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "ResponseBuilder.hpp"

NickCommand::NickCommand() : Command("NICK", std::vector<std::string>()) {}

NickCommand::NickCommand(const NickCommand &other) : Command(other) {}

NickCommand::~NickCommand() {}

NickCommand &NickCommand::operator=(const NickCommand &other)
{
	if (this != &other)
	{
		Command::operator=(other);
	}
	return *this;
}

NickCommand::NickCommand(const std::string &type, const std::vector<std::string> &params) : Command(type, params) {}

bool NickCommand::isValidNick(const std::string &nick) const
{
	if (nick.empty() || nick.length() > 9)
		return false;

	if (std::isdigit(nick[0]) || nick[0] == '-' || nick[0] == '#' || nick[0] == '&' || nick[0] == '+' || nick[0] == '!')
		return false;

	for (size_t i = 0; i < nick.length(); i++)
	{
		char c = nick[i];
		if (!std::isalnum(c) && c != '-' && c != '_' && c != '[' && c != ']' && c != '{' && c != '}' && c != '|' && c != '^' && c != '`' && c != '\\')
			return false;
	}

	return true;
}

void NickCommand::execute(Client *client, Server *server)
{
	ResponseBuilder response;

	// ERR_NOTREGISTERED (451)
	if (!(client->getState() & AUTH_PASS))
	{
		response.prefix(server->getName())
			.numeric(ERR_NOTREGISTERED)
			.target(client->getNick().empty() ? "*" : client->getNick())
			.trailing("You must authenticate with PASS first");

		// Activar POLLOUT y agregar a buffer
		server->queueClientData(*client, response.build());
		return;
	}

	// ERR_NONICKNAMEGIVEN (431)
	if (params_.empty())
	{
		response.prefix(server->getName())
			.numeric(ERR_NONICKNAMEGIVEN)
			.target(client->getNick().empty() ? "*" : client->getNick())
			.trailing("No nickname given");

		std::cerr << "[ircserver]: Error: NICK command without parameters." << std::endl;
		server->queueClientData(*client, response.build());
		return;
	}

	std::string new_nick = params_[0];

	// ERR_ERRONEUSNICKNAME (432)
	if (!isValidNick(new_nick))
	{
		response.prefix(server->getName())
			.numeric(ERR_ERRONEUSNICKNAME)
			.target(new_nick)
			.trailing("Erroneus nickname");

		std::cerr << "[ircserver]: Error: Invalid nickname format: " << new_nick << std::endl;
		server->queueClientData(*client, response.build());
		return;
	}

	std::string old_nick = client->getNick();

	// Si intenta ponerse el mismo nick que ya tiene, no hacemos nada y tampoco devolvemos error
	if (old_nick == new_nick)
		return;

	// ERR_NICKNAMEINUSE (433)
	if (server->isNickInUse(new_nick))
	{
		response.prefix(server->getName())
			.numeric(ERR_NICKNAMEINUSE)
			.target(new_nick)
			.trailing("Nickname is already in use");

		std::cerr << "[ircserver]: Error: Nickname already in use: " << new_nick << std::endl;
		server->queueClientData(*client, response.build());
		return;
	}

	// Si el cliente ya tenía nick y quiere cambiar, quitar el viejo
	if (!old_nick.empty() && old_nick != new_nick)
	{
		server->removeNick(old_nick);
	}

	// Actualizar nick del cliente
	client->setNick(new_nick);
	server->addNick(new_nick);

//	AuthState current_state = client->getState();
	client->setAuthState(AUTH_NICK);
	if (!client->isAuth())				// NUEVO
		server->requestRegistration(*client);

	std::cout << "[ircserver]: Client <" << client->getFd() << ", " << client->getIp()
			  << "> set nick to " << new_nick << std::endl;

	// NO enviar respuesta en caso de éxito (según protocolo IRC)
	return;
}

Command *NickCommand::create(const std::string &type, const std::vector<std::string> &params)
{
	return new NickCommand(type, params);
}
