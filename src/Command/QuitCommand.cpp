#include "Command/QuitCommand.hpp"
#include "Server.hpp"
#include "Channel.hpp"
#include "ResponseBuilder.hpp"

QuitCommand::QuitCommand() : Command() {}

QuitCommand::QuitCommand(const std::string &type, const std::vector<std::string> &params) : Command(type, params) {}

QuitCommand::QuitCommand(const QuitCommand &other) : Command(other) {}

QuitCommand::~QuitCommand() {}

QuitCommand &QuitCommand::operator=(const QuitCommand &other)
{
    if (this != &other)
        Command::operator=(other);
    return *this;
}

void QuitCommand::execute(Client *client, Server *server)
{
    std::string reason = "Client quit";
    if (!params_.empty() && !params_[0].empty())
    {
        reason = params_[0];
    }
    std::string quitMsg = ":" + client->getNick() + "!" + client->getUser() + "@" + client->getIp() + " QUIT :" + reason + "\r\n";
    std::map<std::string, Channel> &channels = server->getChannels(); 
    std::map<std::string, Channel>::iterator it = channels.begin();
    
    std::vector<std::string> channelsToRemove;
    while (it != channels.end())
    {
        Channel &channel = it->second;
        
        if (channel.isMember(client->getFd()))
        {
            channel.broadcastAll(quitMsg, server);
            channel.removeClient(client->getFd());
            
            if (channel.isEmpty())
            {
                channelsToRemove.push_back(it->first);
            }
        }
        ++it;
    }

    for (size_t i = 0; i < channelsToRemove.size(); ++i) //Limpiar los canales vacíos del mapa
    {
        channels.erase(channelsToRemove[i]);
        std::cout << "[ircserver]: Channel " << channelsToRemove[i] << " deleted during QUIT (no members left)." << std::endl;
    }

    client->setToDisconnect(); //desconexion del cliente
    std::cout << "[ircserver]: Client " << client->getNick() << " marked to disconnect via QUIT command." << std::endl;
}

Command *QuitCommand::create(const std::string &type, const std::vector<std::string> &params)
{
    return new QuitCommand(type, params);
}