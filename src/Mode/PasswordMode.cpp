#include "Mode/PasswordMode.hpp"

PasswordMode::PasswordMode() : ModeHandler() {}

PasswordMode::PasswordMode(const PasswordMode &other) : ModeHandler(other)
{
	(void)other;
}

PasswordMode::~PasswordMode() {}

PasswordMode &PasswordMode::operator=(const PasswordMode &other)
{
	if (this != &other)
	{
		ModeHandler::operator=(other);
	}

	return *this;
}

bool PasswordMode::requiresParam(bool isAdding) const
{
	(void)isAdding;
	return true;
}

bool PasswordMode::change(Channel *channel, bool isAdding, const std::string &param, Client *client, Server *server)
{
	(void)isAdding;
	(void)client;
	(void)server;

	if (isAdding && channel->getPassword() == param)
		return false;

	if (!isAdding && channel->getPassword() != param)
		return false;

	channel->setPassword(param);
	return true;
}
