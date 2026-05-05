#ifndef COMMAND_HPP
# define COMMAND_HPP

#include <vector>
#include <string>
#include <stdexcept>
#include <sstream>

enum ECommand {
	UNDEFINED = -1,
	PASS,
	USER,
	NICK
};

class Command
{
private:
	void extractCommandType(std::string &raw_cmd);
	void extractCommandParams(std::string &raw_cmd);

public:
	// Props
	ECommand type;
	std::vector<std::string> params;


	// Forma Canónica Ortodoxa
	Command();
	Command(const Command &other);
	~Command();
	Command &operator=(const Command &other);

	// Create command from raw std::string
	Command(std::string &raw_cmd);
};

#endif // COMMAND_HPP
