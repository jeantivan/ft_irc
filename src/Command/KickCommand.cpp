#include "Command/KickCommand.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "NumericReplies.hpp"

KickCommand::KickCommand() : Command("KICK", std::vector<std::string>()) {}

KickCommand::KickCommand(const std::string &type, const std::vector<std::string> &params) : Command(type, params) {}

KickCommand::KickCommand(const KickCommand &other) : Command(other) {}

KickCommand &KickCommand::operator=(const KickCommand &other)
{
	if (this != &other)
		Command::operator=(other);
	return *this;
}

KickCommand::~KickCommand() {}

Command *KickCommand::create(const std::string &type, const std::vector<std::string> &params)
{
    return new KickCommand(type, params);
}


std::string KickCommand::buildKickMessage(Client *client, const std::string &channelName,
										  const std::string &targetNick, const std::string &reason) const
{
	std::string finalReason = reason.empty() ? "Kicked" : reason;
	return ":" + client->getPrefix() + " KICK " + channelName + " " + targetNick + " :" + finalReason + "\r\n";
}

void KickCommand::execute(Client *client, Server *server)
{
	// KICK <canal> <nick> [:razon]
	if (params_.size() < 2)
	{
		server->sendNumericReply(client, ERR_NEEDMOREPARAMS, "KICK", "Not enough parameters");
		return;
	}

	const std::string channelName = params_[0];
	const std::string targetNick = params_[1];

	Channel *channel = server->getChannel(channelName);
	if (!channel)
	{
		server->sendNumericReply(client, ERR_NOSUCHCHANNEL, channelName, "No such channel");
		return;
	}

	if (!channel->isMember(client->getFd()))
	{
		server->sendNumericReply(client, ERR_NOTONCHANNEL, channelName, "You're not on that channel");
		return;
	}

	if (!channel->isOperator(client->getFd()))
	{
		server->sendNumericReply(client, ERR_CHANOPRIVSNEEDED, channelName, "You're not channel operator");
		return;
	}

	Client *target = server->findClientByNick(targetNick);
	if (!target || !channel->isMember(target->getFd()))
	{
		server->sendNumericReply(client, ERR_USERNOTINCHANNEL, targetNick + " " + channelName, "They aren't on that channel");
		return;
	}

	std::string reason;
	if (params_.size() >= 3)
		reason = params_[2];

	std::string kickMsg = buildKickMessage(client, channelName, targetNick, reason);

	// Primero se notifica a todos, luego se expulsa
	channel->broadcastAll(kickMsg, server);
	channel->removeClient(target->getFd());
}