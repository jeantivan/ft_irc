#include "Client.hpp"

Client::Client() : fd(-1), ip(""), readBuf_(""), writeBuf_(""), authState_(AUTH_NONE), auth_(false), password_(""), nick_(""), user_(""), realname_(""), toDisconnect_(false) {}

Client::Client(const Client &other) : fd(other.fd), ip(other.ip), readBuf_(other.readBuf_), writeBuf_(other.writeBuf_), authState_(other.authState_), auth_(other.auth_), password_(other.password_), nick_(other.nick_), user_(other.user_), realname_(other.realname_), toDisconnect_(other.toDisconnect_) {}

Client &Client::operator=(const Client &other)
{
	if (this != &other)
	{
		fd = other.fd;
		ip = other.ip;
		readBuf_ = other.readBuf_;
		writeBuf_ = other.writeBuf_;
		authState_ = other.authState_;
		auth_ = other.auth_;
		password_ = other.password_;
		nick_ = other.nick_;
		user_ = other.user_;
		realname_ = other.realname_;
		toDisconnect_ = other.toDisconnect_;
	}

	return *this;
}

Client::~Client() {}

Client::Client(int fd, const std::string &ip) : fd(fd), ip(ip), readBuf_(""), writeBuf_(""), authState_(AUTH_NONE), auth_(false), password_(""), nick_(""), user_(""), realname_(""), toDisconnect_(false) {}

int Client::getFd() const
{
	return fd;
}

const std::string &Client::getIp() const
{
	return ip;
}

const std::string &Client::getReadBuf() const
{
	return readBuf_;
}

const std::string &Client::getWriteBuf() const
{
	return writeBuf_;
}

AuthState Client::getState() const
{
	return authState_ ;
}

bool Client::isAuth() const
{
	return (auth_);
}

const std::string &Client::getPassword() const
{
	return password_;
}

const std::string &Client::getNick() const
{
	return nick_;
}

const std::string &Client::getUser() const
{
	return user_;
}

const std::string &Client::getRealname() const
{
	return realname_;
}

bool Client::getToDisconnect() const
{
	return toDisconnect_;
}

void Client::setAuth(bool auth)
{
	auth_ = auth;
}

void Client::setPassword(std::string pass)
{
	password_ = pass;
}

void Client::setUser(const std::string &user)
{
	user_ = user;
}

//En realidad AÑADE estados no cambia quiza mejor nombre addAuthState()
void Client::setAuthState(AuthState adding)
{
	authState_ = static_cast<AuthState>(
    	static_cast<int>(authState_) | static_cast<int>(adding)
    );
}

void Client::setNick(const std::string &nick)
{
	nick_ = nick;
}

void Client::setRealname(const std::string &realname)
{
	realname_ = realname;
}

void Client::setToDisconnect()
{
	toDisconnect_ = true;
}

bool Client::hasCompleteCommand()
{
	return readBuf_.find("\r\n") != std::string::npos;
}

std::string Client::extractCommand()
{
	size_t delim = readBuf_.find("\r\n");

	if (delim == std::string::npos)
		return "";

	std::string cmd = readBuf_.substr(0, delim + 2);
	readBuf_ = readBuf_.substr(delim + 2);

	return cmd;
}

void Client::appendToReadBuf(const char *data, size_t len)
{
	readBuf_.append(data, len);
}

void Client::appendToWriteBuf(const std::string &response)
{
	writeBuf_ += response;
}

void Client::eraseFromWriteBuf(size_t len)
{
	if (len >= writeBuf_.size())
		writeBuf_.clear();
	else
		writeBuf_ = writeBuf_.substr(len);
}
