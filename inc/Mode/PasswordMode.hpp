#ifndef PASSWORDMODE_HPP
#define PASSWORDMODE_HPP

#include "Mode/ModeHandler.hpp"

class PasswordMode : public ModeHandler
{
private:
public:
	PasswordMode();
	PasswordMode(const PasswordMode &other);
	virtual ~PasswordMode();
	PasswordMode &operator=(const PasswordMode &other);

	virtual bool change(Channel *channel, bool isAdding, const std::string &param);

	virtual bool requiresParam(bool isAdding) const;
};

#endif // PASSWORDMODE_HPP
