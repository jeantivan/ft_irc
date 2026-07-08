#include "Server.hpp"
#include "Command/CommandFactory.hpp"
#include "ResponseBuilder.hpp"
#include "Channel.hpp"
#include <ctime>
#include <sstream>

Server::Server() : port_(""), listener_(-1), password_(""), nameServer_(NAME_SERVER), creationDate_(time(NULL)), checkZombiesDate_(creationDate_ + PERIODICCHECK) {}

Server::~Server()
{
	std::cout << "[ircserver]: Shutting down the server" << std::endl;

	for (size_t i = 0; i < connections_.size(); i++)
	{
		int fd = connections_[i].fd;

		if (fd != listener_)
		{
			std::string bye = "Error: Server is shutting down. Goodbye!\r\n";
			send(fd, bye.c_str(), bye.size(), 0);
			close(fd);

			std::cout << "[ircserver]: Client " << fd << " disconnected gracefully." << std::endl;
		}
	}

	if (listener_ != -1)
		close(listener_);

	used_nicks_.clear(); // Limpiar los nicks
}

Server::Server(const Server &other) : port_(other.port_), listener_(other.listener_), password_(other.password_), nameServer_(other.nameServer_), creationDate_(other.creationDate_), checkZombiesDate_(other.checkZombiesDate_) {}

Server &Server::operator=(const Server &other)
{
	if (this != &other)
	{
		port_ = other.port_;
		listener_ = other.listener_;
		password_ = other.password_;
		nameServer_ = other.nameServer_;
		creationDate_ = other.creationDate_;
		checkZombiesDate_ = other.checkZombiesDate_;
	}

	return *this;
}

Server::Server(const char *port, const char *pass) : port_(port), listener_(-1), password_(pass), nameServer_(NAME_SERVER), creationDate_(time(NULL)), used_nicks_(), checkZombiesDate_(creationDate_ + PERIODICCHECK)
{
	init();

	struct pollfd listener_poll;
	listener_poll.fd = listener_;
	listener_poll.events = POLLIN;
	listener_poll.revents = 0;

	connections_.push_back(listener_poll);

	std::cout << "ft_irc: listener created with fd " << listener_ << " on port " << port_ << std::endl;
}

// Getters
std::string Server::getPort() const
{
	return port_;
}

int Server::getListener() const
{
	return listener_;
}

const std::string &Server::getPassword() const
{
	return password_;
}

const std::string &Server::getName() const
{
	return nameServer_;
}

size_t Server::findConnectionByFd(int fd) const
{
	for (size_t i = 0; i < connections_.size(); i++)
	{
		if (connections_[i].fd == fd)
			return i;
	}
	return static_cast<size_t>(-1); // valor maximo, para marcar error
}

void Server::init()
{
	int listener;
	int yes = 1;
	int rv;

	struct addrinfo hints, *ai, *p;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if ((rv = getaddrinfo(NULL, port_.c_str(), &hints, &ai)) != 0)
	{
		throw std::runtime_error("getaddrinfo failed " + std::string(gai_strerror(rv)));
	}

	for (p = ai; p != NULL; p = p->ai_next)
	{
		listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);

		if (listener < 0)
			continue;

		setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

		fcntl(listener, F_SETFL, O_NONBLOCK);

		if (bind(listener, p->ai_addr, p->ai_addrlen) < 0)
		{
			close(listener);
			continue;
		}
		break;
	}

	if (p == NULL)
	{
		throw std::runtime_error("failed to bind. Reason: " + std::string(std::strerror(errno)));
	}

	freeaddrinfo(ai);

	if (listen(listener, 10) == -1)
	{
		throw std::runtime_error("failed to listen");
	}

	listener_ = listener;
}

void Server::run()
{
	std::cout << "[ircserver]: Waiting for connections" << std::endl;
	while (Server::signal_received_ == false)
	{
		int pool_count = poll(&connections_[0], connections_.size(), UNBLOCKPOLL);

		if (pool_count == -1)
		{
			if (errno == EINTR)
				continue;

			throw std::runtime_error("poll failed " + std::string(std::strerror(errno)));
		}
		for (int i = connections_.size() - 1; i >= 0; i--)
		{

			// PLACEHOLDER bloque logica POLLOUT si o no

			if (connections_[i].revents & (POLLIN | POLLHUP))
			{
				if (connections_[i].fd == listener_)
				{
					acceptNewClient();
				}
				else
				{
					receiveClientData(i);
				}
			}
			else if (connections_[i].revents & POLLOUT)
			{
				if (sendClientData(i))
				{
					// TODO: Mejorar mensaje de error
					// std::cerr << "Error: Not all client<" << connections_[i].fd << ", " << clients_[connections_[i].fd].getFd() << "> data could be sent" << std::endl;
				}
			}
		}

		//Limpiamos clientes toDisconnect desde mas de  TEARDOWNTIMEMAX segundos
		if (time(NULL) > checkZombiesDate_)
		{
			dezombify();
			// Programamos proximo checkeo
			checkZombiesDate_ = time(NULL) + PERIODICCHECK;
		
		}
	}
}

void Server::acceptNewClient()
{
	struct sockaddr_storage remoteaddr;
	socklen_t addrlen = sizeof(remoteaddr);
	int new_fd;

	new_fd = accept(listener_, reinterpret_cast<struct sockaddr *>(&remoteaddr), &addrlen);

	if (new_fd == -1)
	{
		throw std::runtime_error("accept failed " + std::string(std::strerror(errno)));
	}

	fcntl(new_fd, F_SETFL, O_NONBLOCK);

	struct pollfd new_connection;

	new_connection.fd = new_fd;
	new_connection.events = POLLIN; // ----- "| POLLOUT" Necesario??
	new_connection.revents = 0;

	connections_.push_back(new_connection);
	std::string remoteIp = getIpStr(reinterpret_cast<struct sockaddr *>(&remoteaddr));

	// TODO: Create Client object and add it to the clients_ map
	clients_[new_fd] = Client(new_fd, remoteIp);

	std::cout << "[ircserver]: New connection from " << remoteIp << " on socket " << new_fd << std::endl;
}

void Server::receiveClientData(size_t client_index)
{
	char buffer[1024];
	std::memset(buffer, 0, sizeof(buffer));

	int client_fd = connections_[client_index].fd;
	Client &client = clients_[client_fd];
	int bytes_received = recv(client_fd, buffer, sizeof(buffer), 0);
	if (bytes_received <= 0)
	{
		if (bytes_received == 0)
		{
			std::cout << "[ircserver]: Client " << client_fd << " disconnected (Closed client end)" << std::endl; // ¿¿¿Es correcto el mensaje???
		}
		else
		{
			std::cerr << "[ircserver]: recv failed on client " << client_fd << " " << std::strerror(errno) << std::endl;
		}


		disconnectClient(client_fd); //esta dejando enlaces colgantes al cliente desconectado en los canales
		//mejor emular un quitcommand, para eliminar al cliente de los canales
		return;
	}

	std::cout << "[ircserver]: Received " << bytes_received << " bytes from client " << client_fd << std::endl;
	client.appendToReadBuf(buffer, bytes_received);

	// TODO: May be this will be deleted
	while (client.hasCompleteCommand() && !client.getToDisconnect()) // && !client.getToDisconnect() evita seguir ejecutando comandos acumulados en buffer de salida, cuando el cliente fué marcado toDisconnect_
	{
		std::string raw_cmd = client.extractCommand();
		std::string type;
		std::vector<std::string> params;

		if (!parse(raw_cmd, type, params))
		{
			std::cerr << "Bad command" << std::endl;
		}

		// WIP: Function to create different commands
		CommandFactory factory;

		Command *cmd = factory.createCommand(type, params); // cmd es obligatoriamente un puntero, es lo que posibilita polimorfismo.
		if (!cmd)
		{
			// TODO: Handle undefined command;
			std::cerr << "Undefined command" << std::endl;
			continue;
		}
		cmd->execute(&client, this);
		delete cmd;
	}
}

void Server::disconnectClient(int fd)
{
	// AÑADIDO NUEVO Liberar nick
	if (clients_.count(fd) > 0)
	{
		std::string nick = clients_[fd].getNick();
		if (!nick.empty())
		{
			removeNick(nick);
			// std::cout << "[ircserver]: Nick '" << nick << "' freed from client " << fd << std::endl;
		}
	}

	//OJOO no esta sacando al cliente de la lista de miembros de los canales TO DO:
	//	- recorrer canales, llamndo a Channel::removeClient(fd)
	//	- borrar canales que queden desiertos a su salida
	//	- tal vez hacer broadcast informando que el cliente salio, en algunos flujos de ejecucion	

	close(fd);

	clients_.erase(fd);

	for (size_t i = 0; i < connections_.size(); i++)
	{
		if (connections_[i].fd == fd)
		{
			connections_.erase(connections_.begin() + i);
			break;
		}
	}
	std::cout << "[ircserver]: Client " << fd << " disconnected (The server did it)" << std::endl;
}

/*
Si se completaron todos los pasos del registro:
- Marca al cliente como autentificado (client.auth_ = true)
- Envia los mensajes de vienvenida RPL_WELCOME, YOURHOST, RPL_CREATED y RPL_MYINFO
Esta funcion debe ser llamada por los comandos USER y NICK (PASS no)
*/
void Server::requestRegistration(Client &client)
{
	if (client.getState() == AUTH_COMPLETE)
	{
		ResponseBuilder response;
		if (password_ == client.getPassword())
		{
			std::cout << "[ircserver]: Client <" << client.getFd() << ", " << client.getIp() << "> is authenticated" << std::endl;
			client.setAuth(true);

			// TODO:extraer este bloque a funcion auxiliar WelcomReply()
			// 001    RPL_WELCOME	"Welcome to the Internet Relay Network <nick>!<user>@<host>"
			response.prefix(getName()).numeric(1).target(client.getNick()).trailing("Welcome to the Internet Relay Network " + client.getNick() + "!" + client.getUser() + "@" + client.getIp());
			queueClientData(client, response.build());
			// 002	YOURHOST		"Your host is <servername>, running version <ver>"
			response.numeric(2).trailing("Your host is " + getName() + ", running version" + SERVER_VERSION);
			queueClientData(client, response.build());
			// 003    RPL_CREATED	"This server was created <date>"
			char date[64];
			struct tm *tm_info = localtime(&creationDate_);
			strftime(date, sizeof(date), "%c", tm_info);
			std::string createdMsg = "This server was created ";
			createdMsg += date;
			response.numeric(3).trailing(createdMsg);
			queueClientData(client, response.build());
			// 004    RPL_MYINFO		"<servername> <version> <available user modes> <available channel modes>"
			response.numeric(4).trailing(nameServer_ + " " + SERVER_VERSION + " " + "io itkol");
			queueClientData(client, response.build());
			// fin WelcomReply()

			return;
		}
		else
		{
			response.prefix(getName()).numeric(ERR_PASSWDMISMATCH).target(client.getNick()).trailing("Password incorrect");
			queueClientData(client, response.build());

			std::cerr << "[ircserver]--->" << client.getFd() << " Error: Password incorrect" << std::endl;
			client.setToDisconnect();
		}
	}
}

// Retorna true si send() falló.
bool Server::sendClientData(size_t client_index)
{
	int fd = connections_[client_index].fd;
	Client &client = clients_[fd];
	const std::string &clientWriteBuf = client.getWriteBuf();

	if (clientWriteBuf.empty()) // Se supone que no llamamos a sendClientData cuando no hay nada que enviar. ¿Es solo defensivo?
	{
		connections_[client_index].events &= ~POLLOUT;
		std::cerr << "[INFO] client in shocket:" << fd << "calls sendClientData wuith writeBuff_ empthy." << std::endl;
		return false;
	}

	ssize_t bytes_sent = send(fd, clientWriteBuf.c_str(), clientWriteBuf.size(), 0);

	if (bytes_sent > 0)
	{
		client.eraseFromWriteBuf(bytes_sent);
		if (client.getWriteBuf().empty())
		{
			connections_[client_index].events &= ~POLLOUT; // "&= ~" borra el bit de POLLOUT
			if (client.getToDisconnect())
				disconnectClient(fd);
		}
	}
	else if (bytes_sent == -1)
	{
		//  TODO: This should not exists because the evals says so
		//		if (errno == EAGAIN || errno == EWOULDBLOCK)
		//		{
		//			return false;
		//		}
		disconnectClient(fd); // pipe roto probablemente??
		return true;
	}
	else
		std::cerr << "[INFO] client in shocket:" << fd << "send() return 0;" << std::endl;
	return false;
}

// TODO: Separar a otro archivo
bool Server::signal_received_ = false;

void Server::signalHandler(int signal)
{
	(void)signal;
	Server::signal_received_ = true;
}

// Añade datos al writeBuf_ de client.
// Activa POLLOUT para que poll() de paso al cliente en cuanto este disponible para recibir datos
// Si el cliente esta marcado toDisconnect_ (teardown), no escribe en su buffer.
void Server::queueClientData(Client &client, const std::string &data)
{
	size_t id = findConnectionByFd(client.getFd());
	if (id == static_cast<size_t>(-1))
	{
		std::cerr << "ERROR en findConnectionByFd" << std::endl;
		// TODO no se si gestionar el error con un throw o como, pero seria grave que quedase asi
		return;
	}
	connections_[id].events |= POLLOUT;
	if (!client.getToDisconnect()) // No queremos seguir metiendo datos en el buffer de un cliente en teardown.
		client.appendToWriteBuf(data);
}

// NICK COMMAND
bool Server::isNickInUse(const std::string &nick) const
{
	return used_nicks_.count(nick) > 0;
}

void Server::addNick(const std::string &nick)
{
	used_nicks_.insert(nick);
}

void Server::removeNick(const std::string &nick)
{
	used_nicks_.erase(nick);
}

// JOIN COMMAND
bool Server::isAchannel(const std::string &channel) const
{
	if (_channels_.find(channel) == _channels_.end())
		return false;
	else
		return true;
}

// Necesario?
Channel *Server::getChannel(const std::string &name)
{
	if (isAchannel(name))
		return &(_channels_[name]);
	return NULL;
}

std::map<std::string, Channel> &Server::getChannels()
{
	return _channels_;
}

bool Server::joinChannel(Client *client, const std::string &nameChannel, const std::string &password)
{
	Channel *channel;
	std::string nickList;
	ResponseBuilder response;

	if (isAchannel(nameChannel))
	{
		if (! _channels_[nameChannel].getPassword().compare(password)) // cuando no haya password estaremos comparando dos strings vacios
		{
			//<client> <channel> :Bad Channel Mask
			sendNumericReply(client, 475, client->getNick() + " " + nameChannel, "Bad Channel Mask");
			std::cout << "[ircserver]:" << client->getNick() << "send JOIN->"
				<< nameChannel << ". But bad passkey" << std::endl;
			return false;
		}

		channel = &(_channels_[nameChannel]);
		// ¿Ya es miembro? Ignorar silenciosamente.
		if (channel->getMembers().find(client->getFd()) != channel->getMembers().end())
		{
			std::cout << "[ircserver]:" << client->getNick() << "send JOIN->"
					  << nameChannel << ". But he was already in the channel" << std::endl;
			return false;
		}
		else // NO es miembro todavía, hacer.
		{
			/////////FASE 6///////////////////////////////////////////////////////////////////////////
			// -¿El canal esta en modo  +k?															//
			//		-Cruza password																	//
			//		-Si el pasword es malo, envia mensaje "475 ERR_BADCHANNELKEY"					//
			// -¿El canal esta en modo +i?															//
			//		-Comprobar invitacion "_chanNames_(chanNames[i]).getPasswd() == channPasword[i]"//
			// 		-Si no lo esta mandar 473 ERR_INVITEONLYCHAN									//
			// -¿El canal alcanzo el numero maximo de usuarios?										//
			// 		-471 ERR_CHANNELISFULL															//
			//////////////////////////////////////////////////////////////////////////////////////////
			if (channel->getMembers().size() > MAX_CHANNEL_MEMBERS) // falta impementar el limite de MODE "L"
			{
				std::cout << "[ircserver]:" << client->getNick() << "send JOIN->"
						  << nameChannel << ". But Channel is full" << std::endl;
				sendNumericReply(client, ERR_CHANNELISFULL, nameChannel, "Cannot join channel (channel is full)");
				return false;
			}

			channel->addClient(client);
			std::cout << "[ircserver]: " << client->getNick() << " Join to: " << nameChannel << std::endl;

			// Cargar (sin enviar) RPL_TOPIC en response.
			response.prefix(getName())
				.numeric(RPL_TOPIC)
				.target(client->getNick())
				.trailing(channel->getTopic());
		}
	}
	else // (El canal no existe)
	{
		// Crear canal
		channel = &createChannel(nameChannel);
		// Añadir client al canal
		channel->addClient(client);
		// Añadir a client como operador al canal
		channel->addOperator(client->getFd());
		// Cargar (sin enviar) RPL_NOTOPIC en response.
		response.prefix(getName())
			.numeric(RPL_NOTOPIC)
			.target(client->getNick())
			.trailing("No topic is set");
	}
	// WELCOME:
	// - Broadcast :<nick>!<user>@<ip> JOIN #canal
	channel->broadcastAll(":" + client->getPrefix() + " JOIN " + nameChannel + "\r\n", this);
	// - RPL_TOPIC o RPL_NOTOPIC.
	queueClientData(*client, response.build());
	// - Envia la lista de miembros con RPL_NAMREPLY y RPL_ENDOFNAMES
	namreply(client, channel);
	return true;
}

void Server::sendNumericReply(Client *client, int numeric, const std::string &params, const std::string &trailing)
{
	ResponseBuilder response;

	response.prefix(getName())
		.numeric(numeric)
		.target(client->getNick());

	if (!params.empty())
		response.params(params);

	if (!trailing.empty())
	{
		response.trailing(trailing);
	}

	queueClientData(*client, response.build());
}

// Lanza uno o mas RPL_NAMEREPLY y un y RPL_ENDOFNAMES al final
void Server::namreply(Client *client, Channel *channel)
{
	std::istringstream nicksStream(channel->getNickList()); // la nicklist deberia incluir "@" delante de cada operador obtener el FD apartir de un nick en este punto del codigo es un dolor
	std::ostringstream paqNicks;
	std::string nick;
	int i = 0;

	while (nicksStream >> nick)
	{
		if (i > 0)
		{
			paqNicks << " ";
		}
		paqNicks << nick;
		i++;

		// ¿Por que 35 Nicks en cada RPL_NAMREPLY?
		//	Longitud maxima de un mensaje 512
		//	cada nick mide como maximo 9 caracteres + @ + " " = 11
		//	(prefix + cmd + (11 * 40) + \r\n) = 512
		//   como no hay necesidad de apurar hasta el limite del protocolo
		//	en lugar de 40 usamos un limite de 35 nicks por respuesta
		if (i == 35)
		{
			sendNumericReply(client, RPL_NAMREPLY, "= " + channel->getName(), paqNicks.str()); // OJO!!! cuando se implementen los modos gestionar "= "

			// Limpieza del stream
			paqNicks.str("");
			paqNicks.clear();
			i = 0;
		}
	}

	if (!paqNicks.str().empty())
	{
		sendNumericReply(client, RPL_NAMREPLY, "= " + channel->getName(), paqNicks.str()); // OJO!!! cuando se implementen los modos gestionar "= "
	}

	// Fin del protocolo (RPL_ENDOFNAMES) usando la MISMA función genérica
	sendNumericReply(client, RPL_ENDOFNAMES, channel->getName(), "End of /NAMES list");
}

// crea un canal, retorna referencia al canl creado
// si el nombre del canal ya estaba en uso, no lo crea, y no falla, retorna referencia ese
// canal.
Channel &Server::createChannel(const std::string &name)
{
	if (_channels_.find(name) != _channels_.end())
		std::cerr << "[ircserver] createChannel: " << name << "cannot be created, it already exists.";
	else
	{
		_channels_[name] = Channel(name);
		// TO DO: instanciar nuevo canal ¿TOPIC o algun otro campo pendiente?

		std::cout << "[ircserver]: Channel " << name << " created" << std::endl;
	}

	return _channels_[name];
}
Client *Server::findClientByNick(const std::string &nick_to_find)
{
	if (nick_to_find.empty())
		return NULL;

	std::map<int, Client>::iterator it;

	for (it = clients_.begin(); it != clients_.end(); ++it)
	{
		if (it->second.getNick() == nick_to_find)
		{
			return &(it->second);
		}
	}
	return NULL;
}

// PART COMMMAND

void Server::leaveChannel(Client *client, const std::string &nameChannel, const std::string &reason)
{
	ResponseBuilder response;
	int clientFd = client->getFd();

	// 1. Validar si el canal existe buscando en el std::map _channels_
	Channel *channelPtr = getChannel(nameChannel); // Seguimos usando tu getChannel seguro
	if (!channelPtr)
	{
		response.prefix(getName())
			.numeric(ERR_NOSUCHCHANNEL)
			.target(client->getNick())
			.trailing(nameChannel + " :No such channel");
		queueClientData(*client, response.build());
		std::cerr << "[ircserver]: Error: ERR_NOSUCHCHANNEL para " << nameChannel << std::endl;
		return;
	}

	Channel &channel = *channelPtr;

	// 2. Validar si el usuario está dentro usando isMember de la clase Channel
	if (!channel.isMember(clientFd))
	{
		response.prefix(getName())
			.numeric(ERR_NOTONCHANNEL)
			.target(client->getNick())
			.params(nameChannel)
			.trailing("You're not on that channel");
		queueClientData(*client, response.build());
		std::cerr << "[ircserver]: Error: ERR_NOTONCHANNEL en " << nameChannel << std::endl;
		return;
	}

	// 3. Construir el mensaje de broadcast oficial de PART
	std::string partMsg = ":" + client->getNick() + "!" + client->getUser() + "@" + client->getIp() + " PART " + nameChannel;
	if (!reason.empty())
		partMsg += " :" + reason;
	partMsg += "\r\n";

	// 4. Enviar el broadcast a TODOS en el canal
	channel.broadcastAll(partMsg, this);

	// 5. Sacar al cliente de la lista de miembros
	channel.removeClient(clientFd);
	std::cout << "[ircserver]: " << client->getNick() << " leave " << nameChannel << "reason" << reason << std::endl;

	// 6. CONTROL DE MEMORIA LIMPIO CON MAPAS (Adiós al bucle for)
	// Si el canal se queda vacío, lo borramos directamente por su clave (nombre)
	if (channel.isEmpty())
	{
		_channels_.erase(nameChannel); // El mapa se encarga de todo en una sola línea
		std::cout << "[ircserver]: Channel " << nameChannel << " deleted de _channels_ (no members left)." << std::endl;
	}
}

void Server::dezombify()
{
    std::map<int, Client>::iterator it = clients_.begin();
    while (it != clients_.end())
    {
        int fd = it->first;
        bool zombie = it->second.getToDisconnect()
            && it->second.getToDisconnectSinze() + TEARDOWNTIMEMAX < time(NULL);
        ++it;

        if (zombie)
            disconnectClient(fd);
    }
}