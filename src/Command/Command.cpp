#include "Command.hpp"
#include <algorithm>

Command::Command() : type_("UNDEFINED"), params_() {}

Command::Command(const Command &other) : type_(other.type_), params_(other.params_) {}

Command::~Command() {}

Command &Command::operator=(const Command &other)
{
	if (this != &other)
	{
		type_ = other.type_;
		params_ = other.params_;
	}

	return *this;
}

Command::Command(const std::string &type, const std::vector<std::string> &params) : type_(type), params_(params) {}

void Command::printCommand()
{
	std::cout << "type: " << type_ << std::endl;
	std::cout << "params: { ";
	if (!params_.empty())
	{
		for (std::vector<std::string>::iterator it = params_.begin(); it != params_.end(); ++it)
		{
			std::cout << "\'" << *it << "\'";
			if (it != params_.end() - 1)
				std::cout << ", ";
		}
	}
	std::cout << " }, params count " << params_.size() << std::endl;
}

// Getters
const std::string &Command::getType() const
{
	return type_;
}

const std::vector<std::string> &Command::getParams() const
{
	return params_;
}
