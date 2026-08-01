#ifndef MODEHANDLER_HPP
#define MODEHANDLER_HPP

#include <string>
#include <iostream>

#include "Channel.hpp"

class ModeHandler
{
private:
public:
	ModeHandler();
	ModeHandler(const ModeHandler &other);
	virtual ~ModeHandler();
	ModeHandler &operator=(const ModeHandler &other);

	virtual bool change(Channel *channel, bool isAdding, const std::string &param, Client *client, Server *server) = 0;
	virtual bool requiresParam(bool isAdding) const = 0;
};

#endif // MODEHANDLER_HPP
