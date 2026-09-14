#include "Server.hpp"
#include "ResponseBuilder.hpp"

void Server::requestRegistration(Client &client)
{
	if (client.getState() == AUTH_COMPLETE)
	{
		ResponseBuilder response;
		if (password_ == client.getPassword())
		{
			std::cout << "[ircserver]: Client <" << client.getFd() << ", " << client.getIp() << "> is authenticated" << std::endl;
			client.setAuth(true);

			// TODO:extraer este bloque a funcion auxiliar WelcomReply()
			// 001    RPL_WELCOME	"Welcome to the Internet Relay Network <nick>!<user>@<host>"
			response.prefix(getName()).numeric(1).target(client.getNick()).trailing("Welcome to the Internet Relay Network " + client.getNick() + "!" + client.getUser() + "@" + client.getIp());
			queueClientData(client, response.build());
			// 002	YOURHOST		"Your host is <servername>, running version <ver>"
			response.numeric(2).trailing("Your host is " + getName() + ", running version " + SERVER_VERSION);
			queueClientData(client, response.build());
			// 003    RPL_CREATED	"This server was created <date>"
			char date[64];
			struct tm *tm_info = localtime(&creationDate_);
			strftime(date, sizeof(date), "%c", tm_info);
			std::string createdMsg = "This server was created ";
			createdMsg += date;
			response.numeric(3).trailing(createdMsg);
			queueClientData(client, response.build());
			// 004    RPL_MYINFO		"<servername> <version> <available user modes> <available channel modes>"
			response.numeric(4).trailing(nameServer_ + " " + SERVER_VERSION + " " + "io itkol");
			queueClientData(client, response.build());
			// fin WelcomReply()

			return;
		}
		else
		{
			response.prefix(getName()).numeric(ERR_PASSWDMISMATCH).target(client.getNick()).trailing("Password incorrect");
			queueClientData(client, response.build());

			std::cerr << "[ircserver]--->" << client.getFd() << " Error: Password incorrect" << std::endl;
			client.setToDisconnect();
		}
	}
}

Client *Server::findClientByNick(const std::string &nick_to_find)
{
	if (nick_to_find.empty())
		return NULL;

	std::map<int, Client>::iterator it;

	for (it = clients_.begin(); it != clients_.end(); ++it)
	{
		if (it->second.getNick() == nick_to_find)
		{
			return &(it->second);
		}
	}
	return NULL;
}

bool Server::isNickInUse(const std::string &nick) const
{
	return used_nicks_.count(nick) > 0;
}

void Server::addNick(const std::string &nick)
{
	used_nicks_.insert(nick);
}

void Server::removeNick(const std::string &nick)
{
	used_nicks_.erase(nick);
}

void Server::queueClientData(Client &client, const std::string &data)
{
	size_t id = findConnectionByFd(client.getFd());
	if (id == static_cast<size_t>(-1))
	{
		std::cerr << "ERROR en findConnectionByFd" << std::endl;
		return;
	}
	connections_[id].events |= POLLOUT;
	if (!client.getToDisconnect())
		client.appendToWriteBuf(data);
}

void Server::sendNumericReply(Client *client, int numeric, const std::string &params, const std::string &trailing)
{
	ResponseBuilder response;

	response.prefix(getName())
		.numeric(numeric)
		.target(client->getNick());

	if (!params.empty())
		response.params(params);

	if (!trailing.empty())
	{
		response.trailing(trailing);
	}

	queueClientData(*client, response.build());
}
