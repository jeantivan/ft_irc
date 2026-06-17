#ifndef CAPCOMMAND_HPP
#define CAPCOMMAND_HPP

#include "Command/Command.hpp"

class CapCommand : public Command
{
private:
public:
	CapCommand();
	CapCommand(const CapCommand &other);
	virtual ~CapCommand();
	CapCommand &operator=(const CapCommand &other);

	CapCommand(const std::string &type, const std::vector<std::string> &params);

	virtual void execute(Client *client, Server *server);

	static Command *create(const std::string &type, const std::vector<std::string> &params);
};

#endif // CAPCOMMAND_HPP
