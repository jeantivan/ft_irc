#include "Command.hpp"
#include <algorithm>

Command::Command() : type("UNDEFINED"), params() {}

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

	parse(raw_cmd);
}

void Command::parse(const std::string &raw_cmd) {
	if (raw_cmd.empty())
		return ;
	
	size_t pos = 0;

	// Skip initial spaces 
	while (pos < raw_cmd.size() && raw_cmd[pos] == ' ')
		pos++;

	// Handle prefix
	if (pos < raw_cmd.size() && raw_cmd[pos] == ':')
	{
		pos = raw_cmd.find(' ', pos);

		while (pos != std::string::npos && pos < raw_cmd.size() && raw_cmd[pos] == ' ')
			pos++;
	}

	if (pos == std::string::npos || pos >= raw_cmd.size()) 
		return ;
	
	// Extract Command type
	size_t cmd_end = raw_cmd.find(' ', pos);
	if (cmd_end == std::string::npos)
	{
		type = raw_cmd.substr(pos);
		return ;
	}

	type = raw_cmd.substr(pos, cmd_end - pos);
	pos = cmd_end;

	// Extract Command params
	while (pos < raw_cmd.size()) 
	{

		while (pos < raw_cmd.size() && raw_cmd[pos] == ' ')
			pos++;

		if (pos >= raw_cmd.size())
			break ;

		if (raw_cmd[pos] == ':')
		{
			params.push_back(raw_cmd.substr(pos + 1));
			break ;
		} else {
			size_t next_space = raw_cmd.find(' ', pos);
			if (next_space == std::string::npos)
			{
				params.push_back(raw_cmd.substr(pos));
				break ;
			} else {
				params.push_back(raw_cmd.substr(pos, next_space - pos));
				pos = next_space;
			}
		}
	}
}

void Command::printCommand( ) {
	std::cout << "type: " << type << std::endl;
	std::cout << "params: { ";
	if (!params.empty()) {
		for (std::vector<std::string>::iterator it = params.begin(); it != params.end(); ++it) {
			std::cout << "\'" << *it << "\'";
			if (it != params.end() - 1)
				std::cout << ", ";
		}
	}
	std::cout << " }, params count " << params.size() << std::endl;
}
