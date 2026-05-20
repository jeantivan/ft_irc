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
};

#endif // RESPONSEBUILDER_HPP
