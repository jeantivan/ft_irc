#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <map>
#include <set>

class Server;
class Client;

class Channel
{
private:
	std::string name_;
	std::string topic_;
	std::map<int, Client *> members_; // fd → Client*
	std::set<int> operators_;		  // fds de operadores

	// Modes
	bool inviteOnly_;			 // i mode
	bool topicRestricted_;		 // t mode
	std::string password_;		 // k mode (empty = no pass)
	unsigned int userLimit_;	 // l mode (0 = no limit)
	std::set<int> invitedUsers_; // Client's fd list if invitedOnly_

public:
	Channel();
	Channel(const std::string &name);
	Channel(const Channel &other);
	Channel &operator=(const Channel &other);
	~Channel();

	// Getters
	const std::string &getName() const;
	const std::string &getTopic() const;
	const std::map<int, Client *> &getMembers() const;
	bool isInviteOnly() const;
	bool isTopicRestricted() const;
	const std::string &getPassword() const;
	unsigned int getUserLimit() const;
	bool isInvited(int fd) const;

	// Setters
	void setTopic(const std::string &topic);
	void setInviteOnly(bool state);
	void setTopicRestricted(bool state);
	void setPassword(const std::string &pass);
	void setUserLimit(unsigned int limit);

	// Miembros
	void addClient(Client *client);
	void removeClient(int fd);
	bool isMember(int fd) const;
	bool isOperator(int fd) const;
	void addOperator(int fd);
	void removeOperator(int fd);
	bool isEmpty() const;
	void addInvited(int fd);
	void removeInvited(int fd);

	// Mensajería
	// Envía a todos los miembros excepto al sender_fd
	void broadcast(const std::string &message, int sender_fd, Server *server) const;
	// Envía a TODOS incluido sender_fd
	void broadcastAll(const std::string &message, Server *server) const;

	// Genera la lista de nicks para RPL_NAMREPLY
	std::string getNickList() const;
};

#endif // CHANNEL_HPP
