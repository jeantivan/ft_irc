#include "Command/TopicCommand.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "ResponseBuilder.hpp"

TopicCommand::TopicCommand() : Command("Topic", std::vector<std::string>()) {}

TopicCommand::TopicCommand(const TopicCommand &other) : Command(other) {}

TopicCommand &TopicCommand::operator=(const TopicCommand &other)
{
	if (this != &other)
	{
		Command::operator=(other);
	}

	return *this;
}

TopicCommand::~TopicCommand() {}

TopicCommand::TopicCommand(const std::string &type, const std::vector<std::string> &params) : Command(type, params) {}

void TopicCommand::execute(Client *client, Server *server)
{
	ResponseBuilder response;
	// response.prefix(server->getName()).numeric(___________).target("*").params("__").trailing("____");
	// std::cerr << "__________" << std::endl;




	server->queueClientData(*client, response.build());
	return;
}

Command *TopicCommand::create(const std::string &type, const std::vector<std::string> &params)
{
	return new TopicCommand(type, params);
}
