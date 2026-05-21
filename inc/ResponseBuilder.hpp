#ifndef RESPONSEBUILDER_HPP
#define RESPONSEBUILDER_HPP

#include <string>
#include <sstream>

class ResponseBuilder
{
private:
	std::string prefix_;
	std::string command_;
	std::string params_;
	std::string trailing_;
	std::string target_;

public:
	ResponseBuilder();
	ResponseBuilder(const ResponseBuilder &other);
	~ResponseBuilder();
	ResponseBuilder &operator=(const ResponseBuilder &other);

	// Build pattern;
	std::string build();
	ResponseBuilder &clear();
	ResponseBuilder &prefix(const std::string &pref);
	ResponseBuilder &command(const std::string &cmd);
	ResponseBuilder &params(const std::string &params);
	ResponseBuilder &trailing(const std::string &trailing);
	ResponseBuilder &target(const std::string &target);
	ResponseBuilder &numeric(int code);
};

#endif // RESPONSEBUILDER_HPP
