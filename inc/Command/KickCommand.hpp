#ifndef KICKCOMMAND_HPP
#define KICKCOMMAND_HPP

#include "Command.hpp"

class KickCommand : public Command
{
public:
	KickCommand();
	KickCommand(const std::string &type, const std::vector<std::string> &params);
	KickCommand(const KickCommand &other);
	KickCommand &operator=(const KickCommand &other);
	virtual ~KickCommand();

	virtual void execute(Client *client, Server *server);

    static Command *create(const std::string &type, const std::vector<std::string> &params);

private:
	std::string buildKickMessage(Client *client, const std::string &channelName,
								 const std::string &targetNick, const std::string &reason) const;
};

#endif