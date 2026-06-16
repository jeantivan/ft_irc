#ifndef NICKCOMMAND_HPP
#define NICKCOMMAND_HPP

#include "Command.hpp"

class NickCommand : public Command
{
private:
	bool isValidNick(const std::string &nick) const;

public:
	NickCommand();
	NickCommand(const NickCommand &other);
	virtual ~NickCommand();
	NickCommand &operator=(const NickCommand &other);

	NickCommand(const std::string &type, const std::vector<std::string> &params);

	virtual void execute(Client *client, Server *server);

	static Command *create(const std::string &type, const std::vector<std::string> &params);
};

#endif // NICKCOMMAND_HPP