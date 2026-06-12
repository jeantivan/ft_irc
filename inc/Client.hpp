#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>
#include <string>
#include <stdexcept>

enum AuthState
{
	AUTH_NONE = 0,		// 0000
	AUTH_PASS = 1 << 0, // 0001 (1)
	AUTH_NICK = 1 << 1, // 0010 (2)
	AUTH_USER = 1 << 2, // 0100 (4)
	AUTH_REGISTERED = 7 // 0111 (1 | 2 | 4)
};

class Client
{
private:
	int fd;
	std::string ip;
	std::string readBuf_;
	std::string writeBuf_;

	AuthState authState_;
	bool auth_; //sin uso, isAuth() hace authState- == AUTH_REGISTERED
	std::string nick_;
	std::string realname_;
	bool toDisconnect_;

public:
	Client();
	Client(const Client &other);
	Client &operator=(const Client &other);
	~Client();

	Client(int fd, const std::string &ip);

	// Getters;
	int getFd() const;
	const std::string &getIp() const;
	const std::string &getReadBuf() const;
	const std::string &getWriteBuf() const;
	bool isAuth() const; //mejor nombre isRegistered()
	AuthState getState() const;
	const std::string &getNick() const;
	const std::string &getRealname() const;
	bool getToDisconnect() const;

	// Setters;
	void setAuthState(AuthState); 
	void setAuth(bool auth);// borrar?
	void setNick(const std::string &nick);
	void setRealname(const std::string &realname);
	void setToDisconnect();

	bool hasCompleteCommand();
	std::string extractCommand();

	void appendToReadBuf(const char *data, size_t len);
	//NO USAR sin gestionar POLLOUT, puedes usar Server::queueueClientData() en su lugar
	void appendToWriteBuf(const std::string &response);
	void eraseFromWriteBuf(size_t len);
};

#endif // CLIENT_HPP
