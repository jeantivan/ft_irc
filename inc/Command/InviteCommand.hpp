#ifndef INVITECOMMAND_HPP
#define INVITECOMMAND_HPP

#include "Command.hpp"

class InviteCommand : public Command
{
public:
	InviteCommand();
	InviteCommand(const std::string &type, const std::vector<std::string> &params);
	InviteCommand(const InviteCommand &other);
	InviteCommand &operator=(const InviteCommand &other);
	virtual ~InviteCommand();

	virtual void execute(Client *client, Server *server);

	static Command *create(const std::string &type, const std::vector<std::string> &params);

private:
	std::string buildInviteMessage(Client *client, const std::string &targetNick,
								   const std::string &channelName) const;
};

#endif