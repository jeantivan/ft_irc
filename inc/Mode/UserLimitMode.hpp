#ifndef USERLIMITMODE_HPP
#define USERLIMITMODE_HPP

#include "Mode/ModeHandler.hpp"

class UserLimitMode : public ModeHandler
{
private:
public:
	UserLimitMode();
	UserLimitMode(const UserLimitMode &other);
	virtual ~UserLimitMode();
	UserLimitMode &operator=(const UserLimitMode &other);

	virtual bool change(Channel *channel, bool isAdding, const std::string &param, Client *client, Server *server);

	virtual bool requiresParam(bool isAdding) const;
};

#endif // USERLIMITMODE_HPP
