#ifndef JOINCOMMAND_HPP
#define JOINCOMMAND_HPP

#include "Command.hpp"
#include "utils.hpp"

class JoinCommand : public Command
{
private:
public:
	JoinCommand();
	JoinCommand(const JoinCommand &other);
	JoinCommand &operator=(const JoinCommand &other);
	JoinCommand(const std::string &type, const std::vector<std::string> &params);
	virtual ~JoinCommand();

	virtual void execute(Client *client, Server *server);

	static Command *create(const std::string &type, const std::vector<std::string> &params);
};

#endif // JOINCOMMAND_HPP
