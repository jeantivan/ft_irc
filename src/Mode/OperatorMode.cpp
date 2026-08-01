#include "Mode/OperatorMode.hpp"

OperatorMode::OperatorMode() : ModeHandler() {}

OperatorMode::OperatorMode(const OperatorMode &other) : ModeHandler(other) {}

OperatorMode::~OperatorMode() {}

OperatorMode &OperatorMode::operator=(const OperatorMode &other)
{
	if (this != &other)
	{
		ModeHandler::operator=(other);
	}
	return *this;
}

bool OperatorMode::requiresParam(bool isAdding) const
{
	(void)isAdding;

	return true;
}

bool OperatorMode::change(Channel *channel, bool isAdding, const std::string &param)
{

	return true;
}
