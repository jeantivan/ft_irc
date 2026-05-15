#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <sstream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <vector>

bool isValidNumber(const std::string &str, int &val);
bool isValidPort(const std::string &str_port, int &port);

std::string getIpStr(const struct sockaddr *sa);
bool parse(std::string &raw_cmd, std::string &type, std::vector<std::string> &params);

#endif // UTILS_HPP
