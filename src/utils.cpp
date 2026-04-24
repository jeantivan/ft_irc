#include "utils.hpp"

bool isValidNumber(const std::string &str, int &val)
{
	int num;
	char extra;
	std::istringstream iss(str);

	if (!(iss >> num) || (iss >> extra))
		return false;
	if (num < 0)
		return false;

	val = num;
	return true;
}

bool isValidPort(const std::string &str_port, int &port)
{

	if (!isValidNumber(str_port, port))
		return false;

	if (port < 1 || port > 65535)
		return false;

	return true;
}
