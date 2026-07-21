#ifndef OPERATORMODE_HPP
#define OPERATORMODE_HPP

#include "Mode/ModeHandler.hpp"

class OperatorMode : public ModeHandler
{
private:
public:
	OperatorMode();
	OperatorMode(const OperatorMode &other);
	virtual ~OperatorMode();
	OperatorMode &operator=(const OperatorMode &other);

	virtual bool change(Channel *channel, bool isAdding, const std::string &param);

	virtual bool requiresParam(bool isAdding) const;
};

#endif // OPERATORMODE_HPP
