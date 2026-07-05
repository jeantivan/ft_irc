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
	AUTH_COMPLETE = 7	// 0111 (1 | 2 | 4) Se instanciaron todos los campos, no significa autentificado
};

class Client
{
private:
	int fd;
	std::string ip;
	std::string readBuf_;
	std::string writeBuf_;

	AuthState authState_;
	bool auth_;
	std::string password_;
	std::string nick_;
	std::string user_;
	std::string realname_;
	bool toDisconnect_;
	time_t lastPong_;

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
	bool isAuth() const;
	const std::string &getPassword() const;
	AuthState getState() const;
	const std::string &getNick() const;
	const std::string &getUser() const;
	const std::string &getRealname() const;
	bool getToDisconnect() const;
	std::string getPrefix() const;
	time_t getLastPong() const;

	// Setters;
	void setAuthState(AuthState);
	void setAuth(bool auth);
	void setPassword(std::string pass);
	void setNick(const std::string &nick);
	void setUser(const std::string &user);
	void setRealname(const std::string &realname);
	/* Pospone la desconexion del cliente, al momento en que se vacie su buffer de salida.
	POLLOUT debe estar activado en pollfd.events para que la desconexion surta efecto*/
	void setToDisconnect();
	void setLastPong(time_t pongRpl);

	bool hasCompleteCommand();
	std::string extractCommand();

	void appendToReadBuf(const char *data, size_t len);
	// NO USAR sin gestionar POLLOUT, puedes usar Server::queueClientData() en su lugar
	void appendToWriteBuf(const std::string &response);
	void eraseFromWriteBuf(size_t len);
};

#endif // CLIENT_HPP
