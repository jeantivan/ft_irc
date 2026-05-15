#ifndef NICKCOMMAND_HPP
#define NICKCOMMAND_HPP

#include "Command/Command.hpp"

class NickCommand : public Command
{
private:
public:
	NickCommand();
	NickCommand(const NickCommand &other);
	virtual ~NickCommand();
	NickCommand &operator=(const NickCommand &other);

	NickCommand::NickCommand(const std::string &type, const std::vector<std::string> &params);

	virtual void execute(Client *client, Server *server);

	static Command *create(const std::string &type, const std::vector<std::string> &params);
};

#endif // NICKCOMMAND_HPP
