#ifndef QUITCOMMAND_HPP
#define QUITCOMMAND_HPP

#include "Command/Command.hpp"

class QuitCommand : public Command
{
public:
    QuitCommand();
    QuitCommand(const std::string &type, const std::vector<std::string> &params);
    QuitCommand(const QuitCommand &other);
    QuitCommand &operator=(const QuitCommand &other);
    virtual ~QuitCommand();

    virtual void execute(Client *client, Server *server);

    static Command *create(const std::string &type, const std::vector<std::string> &params);
};

#endif 