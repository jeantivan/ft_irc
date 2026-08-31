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

bool parse(std::string &raw_cmd, std::string &type, std::vector<std::string> &params)
{
	if (raw_cmd.empty())
		return false;

	// Remove "\r\n" from the raw command
	if (raw_cmd.size() >= 2)
		raw_cmd = raw_cmd.substr(0, raw_cmd.size() - 2);
	else
		raw_cmd.clear();

	size_t pos = 0;

	// Skip initial spaces
	while (pos < raw_cmd.size() && raw_cmd[pos] == ' ')
		pos++;

	// Handle prefix
	if (pos < raw_cmd.size() && raw_cmd[pos] == ':')
	{
		pos = raw_cmd.find(' ', pos);

		while (pos != std::string::npos && pos < raw_cmd.size() && raw_cmd[pos] == ' ')
			pos++;
	}

	if (pos == std::string::npos || pos >= raw_cmd.size())
		return false;

	// Extract Command type
	size_t cmd_end = raw_cmd.find(' ', pos);
	if (cmd_end == std::string::npos)
	{
		type = raw_cmd.substr(pos);
		return true;
	}

	type = raw_cmd.substr(pos, cmd_end - pos);
	pos = cmd_end;

	// Extract Command params
	while (pos < raw_cmd.size())
	{

		while (pos < raw_cmd.size() && raw_cmd[pos] == ' ')
			pos++;

		if (pos >= raw_cmd.size())
			break;

		if (raw_cmd[pos] == ':')
		{
			params.push_back(raw_cmd.substr(pos + 1));
			break;
		}
		else
		{
			size_t next_space = raw_cmd.find(' ', pos);
			if (next_space == std::string::npos)
			{
				params.push_back(raw_cmd.substr(pos));
				break;
			}
			else
			{
				params.push_back(raw_cmd.substr(pos, next_space - pos));
				pos = next_space;
			}
		}
	}
	return true;
}

std::vector<std::string> splitByComma(const std::string &str)
{
	std::istringstream ss(str);
	std::string token;
	std::vector<std::string> tokens;

	while (std::getline(ss, token, ','))
	{
		tokens.push_back(token);
	}

	return tokens;
}
