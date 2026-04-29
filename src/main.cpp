#include "ft_irc.hpp"

int main(int ac, char **av)
{
	if (ac != 3)
	{
		std::cerr << "Error: bad params. Usage: " << av[0] << " <port> <password>" << std::endl;
		return 1;
	}

	std::string str_port = av[1];
	std::string password = av[2];
	int port;

	if (!isValidPort(str_port, port))
	{
		std::cerr << "Error: Invalid port value" << std::endl;
		return 1;
	}

	(void)port;

	try
	{
		Server irc_server(av[1], av[2]);
		irc_server.run();
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}
