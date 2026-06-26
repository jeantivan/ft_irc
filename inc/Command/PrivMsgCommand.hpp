#ifndef PRIVMSGCOMMAND_HPP
#define PRIVMSGCOMMAND_HPP

#include "Command/Command.hpp"

class PrivMsgCommand : Command
{
private:
public:
	PrivMsgCommand();
	PrivMsgCommand(const PrivMsgCommand &other);
	PrivMsgCommand &operator=(const PrivMsgCommand &other);
	PrivMsgCommand(const std::string &type, const std::vector<std::string> &params);
	virtual ~PrivMsgCommand();

	virtual void execute(Client *client, Server *server);

	static Command *create(const std::string &type, const std::vector<std::string> &params);
};

#endif // PRIVMSGCOMMAND_HPP
