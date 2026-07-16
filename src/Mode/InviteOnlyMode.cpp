#include "Mode/InviteOnlyMode.hpp"

InviteOnlyMode::InviteOnlyMode() : ModeHandler() {}

InviteOnlyMode::InviteOnlyMode(const InviteOnlyMode &other) : ModeHandler(other) {}

InviteOnlyMode::~InviteOnlyMode() {}

InviteOnlyMode &InviteOnlyMode::operator=(const InviteOnlyMode &other)
{
	if (this != &other)
	{
		ModeHandler::operator=(other);
	}
	return *this;
}

bool InviteOnlyMode::requiresParam(bool isAdding) const
{
	(void)isAdding;

	return false;
}

bool InviteOnlyMode::change(Channel *channel, bool isAdding, const std::string &param)
{
	(void)param;

	channel->setInviteOnly(isAdding);
	return true;
}
