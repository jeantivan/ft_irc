#include "Mode/OperatorMode.hpp"
#include "Client.hpp"
#include "Server.hpp"
#include "ResponseBuilder.hpp"

OperatorMode::OperatorMode() : ModeHandler() {}

OperatorMode::OperatorMode(const OperatorMode &other) : ModeHandler(other) {}

OperatorMode::~OperatorMode() {}

OperatorMode &OperatorMode::operator=(const OperatorMode &other)
{
	if (this != &other)
	{
		ModeHandler::operator=(other);
	}
	return *this;
}

bool OperatorMode::requiresParam(bool isAdding) const
{
	(void)isAdding;

	return true;
}

bool OperatorMode::change(Channel *channel, bool isAdding, const std::string &param, Client *client, Server *server)
{
	if (param.empty())
		return false;

	const std::map<int, Client *> &members = channel->getMembers();
	std::map<int, Client *>::const_iterator it;
	Client *targetClient = NULL;

	for (it = members.begin(); it != members.end(); ++it)
	{
		if (it->second->getNick() == param)
		{
			targetClient = it->second;
			break;
		}
	}

	if (!targetClient)
	{
		ResponseBuilder response;
		response.prefix(server->getName())
			.numeric(ERR_USERNOTINCHANNEL)
			.target(client->getNick())
			.params(param + " " + channel->getName())
			.trailing("They aren't on that channel");
		std::cout << "[ircserver]: Error ERR_USERNOTINCHANNEL " << response.build() << std::endl;

		server->queueClientData(*client, response.build());
		return false;
	}

	int targetFd = targetClient->getFd();
	bool isCurrentlyOp = channel->isOperator(targetFd);

	if (isAdding && !isCurrentlyOp)
	{
		channel->addOperator(targetFd);
		return true;
	}
	else if (!isAdding && isCurrentlyOp)
	{
		channel->removeOperator(targetFd);
		return true;
	}

	return false;
}
