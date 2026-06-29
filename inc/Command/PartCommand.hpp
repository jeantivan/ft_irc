#ifndef PARTCOMMAND_HPP
#define PARTCOMMAND_HPP

#include "Command/Command.hpp"

class PartCommand : public Command
{
    public:
        PartCommand();
        PartCommand(const std::string &type, const std::vector<std::string> &params);
        PartCommand(const PartCommand &other);
        PartCommand &operator=(const PartCommand &other);
        virtual ~PartCommand();

        void execute(Client *client, Server *server);

        static Command *create(const std::string &type, const std::vector<std::string> &params);
};

#endif