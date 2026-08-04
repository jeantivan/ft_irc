#ifndef MODECOMMAND_HPP
#define MODECOMMAND_HPP

#include "Command/Command.hpp"

class ModeCommand : public Command
{
private:
public:
	ModeCommand();
	ModeCommand(const ModeCommand &other);
	ModeCommand &operator=(const ModeCommand &other);
	ModeCommand(const std::string &type, const std::vector<std::string> &params);
	virtual ~ModeCommand();

	virtual void execute(Client *client, Server *server);

	static Command *create(const std::string &type, const std::vector<std::string> &params);

	void handleChannelMode(Channel *channel, Client *client, Server *server) const;
	void applyChanges(Channel *channel, Client *client, Server *server) const;
};

#endif // MODECOMMAND_HPP
