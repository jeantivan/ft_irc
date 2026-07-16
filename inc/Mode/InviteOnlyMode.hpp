#ifndef INVITEONLYMODE_HPP
#define INVITEONLYMODE_HPP

#include "ModeHandler.hpp"

class InviteOnlyMode : public ModeHandler
{
private:
public:
	InviteOnlyMode();
	InviteOnlyMode(const InviteOnlyMode &other);
	~InviteOnlyMode();
	InviteOnlyMode &operator=(const InviteOnlyMode &other);

	virtual bool change(Channel *channel, bool isAdding, const std::string &param);

	virtual bool requiresParam(bool isAdding) const;
};

#endif // INVITEONLYMODE_HPP
