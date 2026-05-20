#include "ResponseBuilder.hpp"

ResponseBuilder::ResponseBuilder() : prefix_(""), command_(""), params_(""), trailing_(""), target_("") {}

ResponseBuilder::~ResponseBuilder() {}

ResponseBuilder::ResponseBuilder(const ResponseBuilder &other) : prefix_(other.prefix_), command_(other.command_), params_(other.params_), trailing_(other.trailing_), target_(other.target_) {}

ResponseBuilder &ResponseBuilder::operator=(const ResponseBuilder &other)
{
	if (this != &other)
	{
		prefix_ = other.prefix_;
		command_ = other.command_;
		params_ = other.params_;
		trailing_ = other.trailing_;
		target_ = other.target_;
	}
	return *this;
}
