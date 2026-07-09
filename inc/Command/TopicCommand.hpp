#ifndef TOPICCOMMAND_HPP
#define TOPICCOMMAND_HPP

#include "Command.hpp"
#include "utils.hpp"

class TopicCommand : public Command
{
private:
public:
	TopicCommand();
	TopicCommand(const TopicCommand &other);
	TopicCommand &operator=(const TopicCommand &other);
	TopicCommand(const std::string &type, const std::vector<std::string> &params);
	virtual ~TopicCommand();

	virtual void execute(Client *client, Server *server);

	static Command *create(const std::string &type, const std::vector<std::string> &params);
};

#endif // TopicCOMMAND_HPP
