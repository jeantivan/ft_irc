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

/**
 * Convert binary network IP addresses (IPv4 or IPv6)
 * into human-readable text_string.
 *
 * Inspired by: https://beej.us/guide/bgnet/html/split-wide/slightly-advanced-techniques.html
 */
std::string getIpStr(const struct sockaddr *sa)
{
	if (!sa)
		return "";

	char ip[INET6_ADDRSTRLEN];

	switch (sa->sa_family)
	{
	case AF_INET:
	{
		const struct sockaddr_in *sa4 = reinterpret_cast<const struct sockaddr_in *>(sa);

		inet_ntop(AF_INET, &(sa4->sin_addr), ip, sizeof(ip));
		break;
	}
	case AF_INET6:
	{
		const struct sockaddr_in6 *sa6 = reinterpret_cast<const struct sockaddr_in6 *>(sa);

		inet_ntop(AF_INET6, &(sa6->sin6_addr), ip, sizeof(ip));
		break;
	}

	default:
	{
		return "";
	}
	}

	return std::string(ip);
}
