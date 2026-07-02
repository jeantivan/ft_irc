#ifndef PINGCOMMAND_HPP
#define PINGCOMMAND_HPP

#include "Command/Command.hpp"

class PingCommand : public Command
{
private:
public:
	PingCommand();
	PingCommand(const PingCommand &other);
	virtual ~PingCommand();
	PingCommand &operator=(const PingCommand &other);
	PingCommand(const std::string &type, const std::vector<std::string> &params);

	virtual void execute(Client *client, Server *server);
	static Command *create(const std::string &type, const std::vector<std::string> &params);
};

#endif // PINGCOMMAND_HPP
