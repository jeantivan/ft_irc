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
};

#endif // MODECOMMAND_HPP
