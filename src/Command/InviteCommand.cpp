#include "Command/InviteCommand.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "NumericReplies.hpp"

InviteCommand::InviteCommand() : Command("INVITE", std::vector<std::string>()) {}

InviteCommand::InviteCommand(const std::string &type, const std::vector<std::string> &params) : Command(type, params) {}

InviteCommand::InviteCommand(const InviteCommand &other) : Command(other) {}

InviteCommand &InviteCommand::operator=(const InviteCommand &other)
{
	if (this != &other)
		Command::operator=(other);
	return *this;
}

InviteCommand::~InviteCommand() {}

Command *InviteCommand::create(const std::string &type, const std::vector<std::string> &params)
{
    return new InviteCommand(type, params);
}

std::string InviteCommand::buildInviteMessage(Client *client, const std::string &targetNick,
											  const std::string &channelName) const
{
	return ":" + client->getPrefix() + " INVITE " + targetNick + " " + channelName + "\r\n";
}

void InviteCommand::execute(Client *client, Server *server)
{
	// INVITE <nick> <canal>
	if (params_.size() < 2)
	{
		server->sendNumericReply(client, ERR_NEEDMOREPARAMS, "INVITE", "Not enough parameters");
		return;
	}

	const std::string targetNick = params_[0];
	const std::string channelName = params_[1];

	Client *target = server->findClientByNick(targetNick);
	if (!target)
	{
		server->sendNumericReply(client, ERR_NOSUCHNICK, targetNick, "No such nick/channel");
		return;
	}

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

	// Si canal +i, solo operadores invitan 
	if (channel->isInviteOnly() && !channel->isOperator(client->getFd()))
	{
		server->sendNumericReply(client, ERR_CHANOPRIVSNEEDED, channelName, "You're not channel operator");
		return;
	}

	if (channel->isMember(target->getFd()))
	{
		// 443: <nick> <channel> :is already on channel
		server->sendNumericReply(client, ERR_USERONCHANNEL, targetNick + " " + channelName, "is already on channel");
		return;
	}

	// depende de cambios de A
	channel->addInvited(target->getFd());

	server->sendNumericReply(client, RPL_INVITING, targetNick + " " + channelName, "");

	// mensaje INVITE al target
	server->queueClientData(*target, buildInviteMessage(client, targetNick, channelName));
}