#include "Mode/ModeHandler.hpp"

ModeHandler::ModeHandler() {}

ModeHandler::ModeHandler(const ModeHandler &other)
{
	(void)other;
}

ModeHandler::~ModeHandler() {}

ModeHandler &ModeHandler::operator=(const ModeHandler &other)
{
	if (this != &other)
	{
	}
	return *this;
}
