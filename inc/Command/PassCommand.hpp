#ifndef PASSCOMMAND_HPP
#define PASSCOMMAND_HPP

#include "Command.hpp"

class PassCommand : public Command
{
private:
public:
	PassCommand();
	PassCommand(const PassCommand &other);
	PassCommand &operator=(const PassCommand &other);
	PassCommand(const std::string &type, const std::vector<std::string> &params);
	virtual ~PassCommand();

	virtual void execute(Client *client, Server *server);

	static Command *create(const std::string &type, const std::vector<std::string> &params);
};

#endif // PASSCOMMAND_HPP
