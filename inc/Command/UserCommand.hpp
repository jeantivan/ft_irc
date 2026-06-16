#ifndef USERCOMMAND_HPP
#define USERCOMMAND_HPP

#include "Command.hpp"

class UserCommand : public Command
{
private:
public:
	UserCommand();
	UserCommand(const UserCommand &other);
	UserCommand &operator=(const UserCommand &other);
	UserCommand(const std::string &type, const std::vector<std::string> &params);
	virtual ~UserCommand();

	virtual void execute(Client *client, Server *server);

	static Command *create(const std::string &type, const std::vector<std::string> &params);
};

#endif // USERCOMMAND_HPP
