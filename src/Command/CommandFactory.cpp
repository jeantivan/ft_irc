#include <Command/CommandFactory.hpp>
#include <Command/PassCommand.hpp>
#include <Command/NickCommand.hpp>

CommandFactory::CommandFactory()
{
	creators_["PASS"] = &PassCommand::create;
	creators_["NICK"] = &NickCommand::create;
}

CommandFactory::~CommandFactory() {}

CommandFactory::CommandFactory(const CommandFactory &other) : creators_(other.creators_) {}

CommandFactory &CommandFactory::operator=(const CommandFactory &other)
{
	if (this != &other)
	{
		creators_ = other.creators_;
	}

	return *this;
}

Command *CommandFactory::createCommand(const std::string &type, const std::vector<std::string> &params)
{
	std::map<std::string, CommandCreator>::iterator it = creators_.find(type);
	std::cerr << "Command receive " << type << std::endl;
	if (it != creators_.end())
	{
		return it->second(type, params);
	}
	return NULL;
}
