*This project has been created as part
of the 42 curriculum by jtivan-r, carmelag, antofern.*

# Description
ft_irc is a basic IRC (Internet Relay Chat) server.
It allows direct communication between clients as well as communication
through channels. It has been built following rfc1459 for its core
behaviour, using irssi as the reference client.
It implements the commands INVITE, JOIN, KICK, MODE, NICK, PART, PASS, QUIT,
TOPIC and USER. It also accepts PING, and handles CAP by informing the client
that capability negotiation is not supported.

# Instructions
### Starting the server:
1. Compile with ```make``` in the project folder.
2. Run ```./ircserv 6667 <PASS>```. The standard port for the protocol is
6667, but you can use a different port instead. PASS is the password that
clients are required to send in order to complete the registration process.
### Connecting as a client
#### With irssi
##### Running irssi (ignoring the client's internal configuration)
- ```irssi -c localhost -p 6667 -w <PASSWORD> -n <nick>```. If you are
connecting from a different machine, use the server's IP instead of
localhost.
##### Configuring the client from interactive mode:
1. Create a network:
```/network add ircserv```
2. Add our server to the network:
``` /server add -network ircserv localhost 6667 abc```
3. Set a nick, username, and realname:
	```
	/set nick <nick>
	/set user_name <user>
	/set real_name "<full name>"
	```
4. Connect:
```/connect ircserv```

#### With Netcat
For finer control over the packets being sent you can use netcat:
```nc localhost 6667``` type '\r' before a newline to signal the end of a
command.
```nc -C localhost 6667``` -C automatically inserts '\r' on every newline.
### Client usage
Since irssi auto-completes the registration process, once connected you can
send a direct message to another user with ```msg recipient-nick message```,
or create/join a channel with ```/join channelname```.
When you are joined to several channels or private conversations you can
switch between them with ALT+n (1, 2, 3, 4, 5, ...).
To leave a channel or conversation: ```/part channelname```.

* [PASS](https://www.google.com/search?q=https://irssi.org/documentation/help/1.4/pass/) sends the password to the server.
* [NICK](https://irssi.org/documentation/help/1.4/nick/) sets a nickname for the client.
* [USER](https://www.google.com/search?q=https://irssi.org/documentation/help/1.4/user/) sets the username/realname.
* [INVITE](https://irssi.org/documentation/help/1.4/invite/) allows a user to join an invite-only channel.
* [JOIN](https://irssi.org/documentation/help/1.4/join/) joins a user to a channel, provided they have the required permissions.
* [KICK](https://irssi.org/documentation/help/1.4/kick/) ejects another user from a channel.
* [PART](https://irssi.org/documentation/help/1.4/part/) makes the user leave the given channels, with an optional farewell message.
* [QUIT](https://irssi.org/documentation/help/1.4/quit/) leaves every channel and closes the client.
* [TOPIC](https://irssi.org/documentation/help/1.4/topic/) shows/changes the channel's information.
* [MODE](https://irssi.org/documentation/help/1.4/mode/)
	- i Enables invite-only mode.
	- t Prevents non-operator users from changing the channel topic.
	- k Protects a channel with a key.
	- o Grants operator status to a user.
	- l Sets a maximum number of users.

# Resources
- Reference document for the IRC protocol https://www.rfc-editor.org/info/rfc1459/
- A clearer reference focused on client communication https://modern.ircdocs.horse/
irssi user manual https://irssi.org/documentation/manual/
- AI tools were used only to help research and understand theoretical concepts
around the project.
They were also used to translate this document from Spanish.

# Technical Choices
- **Non-blocking I/O:** As required by the subject, we use a single thread that services all connections by iterating over each of them whenever `poll()` reports an event. Sockets are set to `O_NONBLOCK`, preventing a slow client from blocking everyone else in a call to `recv()`, `send()` or `accept()`. `POLLOUT` is only enabled when the server has queued some response for the client.
- **Read buffer per client:** each `Client` has a read buffer, since we cannot know whether a call to `recv()` will bring one command, several commands, or an incomplete one.
- **Write buffer per client:** allows queuing responses and sending them once the client is ready.
- **Polymorphism in Command:** commands (JOIN, PART, PRIVMSG, MODE, etc.) are classes that inherit from `Command`, instantiated by `CommandFactory`. This enables runtime polymorphism, i.e. we can call the specific `execute()` for the command type even when we don't know in advance which command will arrive.
- **Manual memory management:** `Client`, `Channel` and `Command` objects are created and destroyed according to the execution flow, to avoid memory leaks.
