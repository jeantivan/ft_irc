#ifndef TOPICRESTRICTEDMODE_HPP
#define TOPICRESTRICTEDMODE_HPP

#include "ModeHandler.hpp"

class TopicRestrictedMode : public ModeHandler
{
private:
public:
	TopicRestrictedMode();
	TopicRestrictedMode(const TopicRestrictedMode &other);
	virtual ~TopicRestrictedMode();
	TopicRestrictedMode &operator=(const TopicRestrictedMode &other);

	virtual bool change(Channel *channel, bool isAdding, const std::string &param);

	virtual bool requiresParam(bool isAdding) const;
};

#endif // TOPICRESTRICTEDMODE_HPP
