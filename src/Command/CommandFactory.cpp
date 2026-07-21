#include <Command/CommandFactory.hpp>
#include <Command/PassCommand.hpp>
#include <Command/UserCommand.hpp>
#include <Command/NickCommand.hpp>
#include <Command/CapCommand.hpp>
#include "Command/JoinCommand.hpp"
#include <Command/PrivMsgCommand.hpp>
#include <Command/PartCommand.hpp>
#include <Command/QuitCommand.hpp>
#include <Command/PingCommand.hpp>
#include <Command/UnknownCommand.hpp>
#include <Command/ModeCommand.hpp>

CommandFactory::CommandFactory()
{
	creators_["PASS"] = &PassCommand::create;
	creators_["USER"] = &UserCommand::create;
	creators_["NICK"] = &NickCommand::create;
	creators_["CAP"] = &CapCommand::create;
	creators_["JOIN"] = &JoinCommand::create;
	creators_["PRIVMSG"] = &PrivMsgCommand::create;
	creators_["PART"] = &PartCommand::create;
	creators_["QUIT"] = &QuitCommand::create;
	creators_["PING"] = &PingCommand::create;
	creators_["MODE"] = &ModeCommand::create;
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
	Command *unknown = UnknownCommand::create(type, params);
	return unknown;
}
