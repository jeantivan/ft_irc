#include "Mode/UserLimitMode.hpp"
#include <cstdlib>

UserLimitMode::UserLimitMode() : ModeHandler() {}

UserLimitMode::UserLimitMode(const UserLimitMode &other) : ModeHandler(other)
{
	(void)other;
}

UserLimitMode::~UserLimitMode() {}

UserLimitMode &UserLimitMode::operator=(const UserLimitMode &other)
{
	if (this != &other)
	{
		ModeHandler::operator=(other);
	}

	return *this;
}

bool UserLimitMode::requiresParam(bool isAdding) const
{
	return isAdding;
}

bool UserLimitMode::change(Channel *channel, bool isAdding, const std::string &param)
{
	(void)param;

	if (isAdding)
	{
		if (param.empty())
			return false;

		int limit = std::atoi(param.c_str());
		if (limit <= 0)
			return false;

		if (static_cast<unsigned int>(limit) == channel->getUserLimit())
			return false;
		channel->setUserLimit(limit);
	}
	else
	{
		channel->setUserLimit(0);
	}

	return true;
}
