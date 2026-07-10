#include "Command/TopicCommand.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "ResponseBuilder.hpp"

TopicCommand::TopicCommand() : Command("TOPIC", std::vector<std::string>()) {}

TopicCommand::TopicCommand(const TopicCommand &other) : Command(other) {}

TopicCommand &TopicCommand::operator=(const TopicCommand &other)
{
	if (this != &other)
	{
		Command::operator=(other);
	}

	return *this;
}

TopicCommand::~TopicCommand() {}

TopicCommand::TopicCommand(const std::string &type, const std::vector<std::string> &params) : Command(type, params) {}

void TopicCommand::execute(Client *client, Server *server)
{
	ResponseBuilder response;
	int clientFd = client->getFd();

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

	if (params_.size() == 0)
	{
		server->sendNumericReply(client, ERR_NEEDMOREPARAMS, "TOPIC", "Not enough parameters");
		return;
	}

	Channel *channel = server->getChannel(params_[0]);
	//Verifica si el canal existe.
	if (channel == NULL)
	{
		server->sendNumericReply(client, ERR_NOSUCHCHANNEL, params_[0], "No such channel");
		return;
	}
	std::string channName = channel->getName();

// Consulta del Tema Actual (Un solo parámetro)
	if (params_.size() == 1)
	{
		/*	"If the client sending this command is not joined to the given channel, and tries to view
			its’ topic, the server MAY return the ERR_NOTONCHANNEL (442) numeric and have the command
			fail."
		-Ese MAY implica que podemos implementar que el topic sea accesible para todos o solo para
		miembros. Me he quedado con solo para miembros. */
		if(!channel->isMember(clientFd))
		{
			server->sendNumericReply(client, ERR_NOTONCHANNEL, channName, "You're not on that channel");
			return;
		}
		std::string topic = channel->getTopic();
		//Si el canal NO tiene tema: El servidor responde con RPL_NOTOPIC (331) (ej. #canal :No topic is set).
		if (topic.empty())
		{
			server->sendNumericReply(client, RPL_NOTOPIC, channName, "No topic is set");
			return;
		}

		//Si el canal tiene tema: El servidor responde con el valor numérico RPL_TOPIC (332)
		server->sendNumericReply(client, RPL_TOPIC, channName, topic);
		server->sendNumericReply(client, RPL_TOPICWHOTIME, channel->getTopicAuthor() + " " + channel->getTopicTime(), "");
		return;
	}
// Modificación del Tema (Dos parámetros)
	if (params_.size() > 1) 
	{
		if(!channel->isMember(clientFd))
		{
			server->sendNumericReply(client, ERR_NOTONCHANNEL, channName, "You're not on that channel");
			return;
		}

		// Si el canal tiene el modo +t activo: Solo los operadores del canal (@) pueden cambiar el tema. Si un usuario normal lo intenta, el servidor deniega la acción y devuelve ERR_CHANOPRIVSNEEDED (482).
		if (channel->isTopicRestricted() && !channel->isOperator(clientFd))
		{
			server->sendNumericReply(client, ERR_CHANOPRIVSNEEDED, channName, "You're not channel operator");
			return;
		}

		// Aplicación del cambio: El servidor actualiza el string del tema en la memoria/base de datos, guarda el nickname del autor y el timestamp actual.
		channel->setTopic(params_[1], client->getNick());
		// Broadcast con nuevo topic:
		response.prefix(client->getPrefix()).command("TOPIC").target(channName).trailing(params_[1]);
		channel->broadcastAll(response.build(), server);
	}
	return;
}

Command *TopicCommand::create(const std::string &type, const std::vector<std::string> &params)
{
	return new TopicCommand(type, params);
}
