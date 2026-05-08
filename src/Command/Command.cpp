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

Command::Command(const std::string &type, const std::vector<std::string> &params) : type(type), params(params) {}

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
