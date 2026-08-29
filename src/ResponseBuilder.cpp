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

ResponseBuilder &ResponseBuilder::clear()
{
	prefix_ = "";
	command_ = "";
	params_ = "";
	trailing_ = "";
	target_ = "";
	return *this;
}

ResponseBuilder &ResponseBuilder::prefix(const std::string &pref)
{
	prefix_ = pref;
	return *this;
}

ResponseBuilder &ResponseBuilder::command(const std::string &cmd)
{
	command_ = cmd;
	return *this;
}

ResponseBuilder &ResponseBuilder::params(const std::string &params)
{
	if (!params_.empty())
		params_ += " ";
	params_ += params;
	return *this;
}

ResponseBuilder &ResponseBuilder::target(const std::string &target)
{
	target_ = target;
	return *this;
}

// AVISO: llamar a  ResponseBuilder::trailing("") con una cadena vacia, introducirá " :" al final
// de ResponseBuilder.build().
// Esos ":" finales marcan que un argumento existe y esta vacío, que no es lo mismo que
// no tener argumento.
// Por ejemplo, cuando queremos borrar el topic de un canal
// mandamos ":user!username@ip TOPIC #test :"

ResponseBuilder &ResponseBuilder::trailing(const std::string &trailing)
{
	trailing_ = " :" + trailing;
	return *this;
}

ResponseBuilder &ResponseBuilder::numeric(int code)
{
	std::ostringstream oss;
	if (code < 10)
	{
		oss << "00" << code;
	}
	else if (code < 100)
	{
		oss << "0" << code;
	}
	else
		oss << code;
	command_ = oss.str();
	return *this;
}

const std::string ResponseBuilder::build() const
{
	std::string result;

	if (!prefix_.empty())
		result += ":" + prefix_ + " ";
	result += command_;

	if (!target_.empty())
		result += " " + target_;

	if (!params_.empty())
		result += " " + params_;

	if (!trailing_.empty())
		result += trailing_;//trailing() ya introdujo " :"

	result += "\r\n";

	return result;
}
