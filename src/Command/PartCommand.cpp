#include "Command/PartCommand.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "ResponseBuilder.hpp"
#include "NumericReplies.hpp"

PartCommand::PartCommand() : Command("PART", std::vector<std::string>()) {}

PartCommand::PartCommand(const PartCommand &other) : Command(other) {}

PartCommand &PartCommand::operator=(const PartCommand &other)
{
    if (this != &other)
    {
        Command::operator=(other);
    }
    return *this;
}

PartCommand::~PartCommand() {}

PartCommand::PartCommand(const std::string &type, const std::vector<std::string> &params) : Command(type, params) {}

Command *PartCommand::create(const std::string &type, const std::vector<std::string> &params)
{
    return new PartCommand(type, params);
}

void PartCommand::execute(Client *client, Server *server)
{
    ResponseBuilder response;

    if (!client->isAuth())
    {
        response.prefix(server->getName())
            .numeric(ERR_NOTREGISTERED)
            .target(client->getNick())
            .trailing("You have not registered");
        server->queueClientData(*client, response.build());
        std::cerr << "[ircserver]: Error: ERR_NOTREGISTERED en PART" << std::endl;
        return;
    }

    if (params_.size() < 1)
    {
        response.prefix(server->getName())
            .numeric(ERR_NEEDMOREPARAMS)
            .target(client->getNick())
            .trailing("Not enough parameters");
        server->queueClientData(*client, response.build());
        std::cerr << "[ircserver]: Error: ERR_NEEDMOREPARAMS en PART" << std::endl;
        return;
    }

    std::vector<std::string> chanNames = splitByComma(params_[0]);

    std::string reason = "";
    if (params_.size() >= 2)
    {
        reason = params_[1];
    }

    for (size_t i = 0; i < chanNames.size(); i++)
    {
        server->leaveChannel(client, chanNames[i], reason);
    }
}
