#include "Mode/TopicRestrictedMode.hpp"

TopicRestrictedMode::TopicRestrictedMode() : ModeHandler() {}

TopicRestrictedMode::TopicRestrictedMode(const TopicRestrictedMode &other) : ModeHandler(other)
{
	(void)other;
}

TopicRestrictedMode::~TopicRestrictedMode() {}

TopicRestrictedMode &TopicRestrictedMode::operator=(const TopicRestrictedMode &other)
{
	if (this != &other)
	{
		ModeHandler::operator=(other);
	}

	return *this;
}

bool TopicRestrictedMode::requiresParam(bool isAdding) const
{
	(void)isAdding;
	return false;
}

bool TopicRestrictedMode::change(Channel *channel, bool isAdding, const std::string &params)
{
	(void)params;

	if (channel->isTopicRestricted() == isAdding)
		return false;

	channel->setTopicRestricted(isAdding);
	return true;
}
