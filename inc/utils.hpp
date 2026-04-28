#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <sstream>
#include <sys/socket.h>
#include <arpa/inet.h>

bool isValidNumber(const std::string &str, int &val);
bool isValidPort(const std::string &str_port, int &port);

std::string getIpStr(const struct sockaddr *sa);

#endif // UTILS_HPP
