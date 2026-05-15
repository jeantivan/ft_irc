#ifndef COMMANDFACTORY_HPP
#define COMMANDFACTORY_HPP

#include "Command/Command.hpp"
#include <map>
#include <string>

// Factory
typedef Command *(*CommandCreator)(const std::string &type, const std::vector<std::string> &params);

class CommandFactory
{
private:
	std::map<std::string, CommandCreator> creators_;

public:
	CommandFactory();
	CommandFactory(const CommandFactory &other);
	~CommandFactory();
	CommandFactory &operator=(const CommandFactory &other);

	Command *createCommand(const std::string &type, const std::vector<std::string> &params);
};

#endif // COMMANDFACTORY_HPP
