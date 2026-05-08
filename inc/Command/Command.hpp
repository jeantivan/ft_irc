#ifndef COMMAND_HPP
#define COMMAND_HPP

#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <sstream>

class Server;
class Client;

class Command
{
protected:
	// Props
	std::string type;
	std::vector<std::string> params;

public:
	// Forma Canónica Ortodoxa
	Command();
	Command(const Command &other);
	virtual ~Command();
	Command &operator=(const Command &other);

	Command(const std::string &type, const std::vector<std::string> &params);

	virtual void execute(Client *client, Server *server) = 0;

	void printCommand();
};

#endif // COMMAND_HPP
