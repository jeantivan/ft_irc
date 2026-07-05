#ifndef PONGCOMMAND_HPP
#define PONGCOMMAND_HPP

#include "Command/Command.hpp"

class PongCommand : public Command
{
private:
public:
	PongCommand();
	PongCommand(const PongCommand &other);
	virtual ~PongCommand();
	PongCommand &operator=(const PongCommand &other);
	PongCommand(const std::string &type, const std::vector<std::string> &params);

	virtual void execute(Client *client, Server *server);
	static Command *create(const std::string &type, const std::vector<std::string> &params);
};

#endif // PONGCOMMAND_HPP
