#ifndef UNKNOWNCOMMAND_HPP
#define UNKNOWNCOMMAND_HPP

#include "Command/Command.hpp"

class UnknownCommand : public Command
{
private:
public:
	UnknownCommand();
	UnknownCommand(const UnknownCommand &other);
	virtual ~UnknownCommand();
	UnknownCommand(const std::string &type, const std::vector<std::string> &params);
	UnknownCommand &operator=(const UnknownCommand &other);

	virtual void execute(Client *client, Server *server);

	static Command *create(const std::string &type, const std::vector<std::string> &params);
};

#endif // UNKNOWNCOMMAND_HPP
