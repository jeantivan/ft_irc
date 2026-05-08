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
	std::string type_;
	std::vector<std::string> params_;

public:
	// Forma Canónica Ortodoxa
	Command();
	Command(const Command &other);
	virtual ~Command();
	Command &operator=(const Command &other);

	Command(const std::string &type, const std::vector<std::string> &params);

	// TODO: Convertir en Abstracta y crear las derivadas.
	virtual void execute(Client *client, Server *server); //= 0;

	void printCommand();

	// Getters
	const std::string &getType() const;
	const std::vector<std::string> &getParams() const;
};

#endif // COMMAND_HPP
