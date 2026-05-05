#include "Command.hpp"

Command::Command() : type(ECommand::UNDEFINED), params() {}

Command::Command(const Command &other) : type(other.type), params(other.params) {}

Command::~Command() {}

Command &Command::operator=(const Command &other) {
	if (this != &other)
	{
		type = other.type;
		params = other.params;
	}

	return *this;
}

Command::Command(std::string &raw_cmd) {
	// Delete the "\r\n"
	raw_cmd = raw_cmd.substr(0, raw_cmd.size() - 2);

	extractCommandType(raw_cmd);

	extractCommandParams(raw_cmd);
}

void Command::extractCommandType(std::string &raw_cmd) {
	std::stringstream ss(raw_cmd);

	std::string cmd_str;

	ss >> cmd_str;

	if (cmd_str == ":")
		ss >> cmd_str;

	// TODO: Hacer algo mas elegante?
	if (cmd_str == "PASS")
		type = ECommand::PASS;
	else if (cmd_str == "NICK")
		type = ECommand::NICK;
	else if (cmd_str == "USER")
		type = ECommand::USER;
	else
		type = ECommand::UNDEFINED;
}
