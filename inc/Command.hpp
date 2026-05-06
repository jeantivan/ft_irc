#ifndef COMMAND_HPP
# define COMMAND_HPP

#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <sstream>

class Command
{
private:
	void parse(const std::string &raw_cmd);

public:
	// Props
	std::string type;
	std::vector<std::string> params;


	// Forma Canónica Ortodoxa
	Command();
	Command(const Command &other);
	~Command();
	Command &operator=(const Command &other);

	// Create command from raw std::string
	Command(std::string &raw_cmd);

	void printCommand();
};

#endif // COMMAND_HPP
