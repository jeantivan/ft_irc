# FASE 5 — Canales y Mensajería: Guía técnica y reparto de tareas

> Basado en vuestro código actual (Fases 1-4 completas): arquitectura con `CommandFactory`, patrón `Command` polimórfico, `ResponseBuilder`, `queueClientData()` + POLLOUT dinámico, y `disconnectClient()` por fd.

---

## Parte I — Referencia técnica de comandos

---

### 1. `JOIN` — Unirse o crear un canal

#### Sintaxis
```
JOIN #canal
JOIN #canal clave
JOIN 0          ← caso especial: salir de TODOS los canales
```

#### Flujo de ejecución
1. Verificar que el cliente está registrado (`client->isAuth()`). Si no → `ERR_NOTREGISTERED (451)`.
2. Verificar que hay al menos un parámetro. Si no → `ERR_NEEDMOREPARAMS (461)`.
3. Validar que el nombre empieza por `#` y tiene entre 2 y 50 caracteres sin espacios ni `\a` ni `,`. Si no → `ERR_NOSUCHCHANNEL (403)`.
4. Buscar el canal en `_channels` del Server:
   - **No existe** → crearlo, añadir al cliente como miembro Y como operador (`_operators`).
   - **Existe** → comprobar modos (más adelante, Fase 6). Por ahora, simplemente añadir al cliente.
5. Verificar que el cliente no está ya en el canal. Si ya está → ignorar silenciosamente (no es un error según RFC).
6. Enviar respuestas de éxito (ver abajo).

#### Respuestas de éxito
Todas estas respuestas se envían **en este orden**:

```
# 1. Notificar a TODOS los miembros del canal (incluido el nuevo) el JOIN
:<nick>!<user>@<ip> JOIN #canal

# 2. Enviar el topic al nuevo miembro (si el canal no tiene topic, se usa RPL_NOTOPIC)
:servidor 332 <nick> #canal :Texto del topic       ← RPL_TOPIC (332)
:servidor 331 <nick> #canal :No topic is set       ← RPL_NOTOPIC (331)

# 3. Lista de usuarios del canal → RPL_NAMREPLY (353) + RPL_ENDOFNAMES (366)
:servidor 353 <nick> = #canal :@operador user1 user2
:servidor 366 <nick> #canal :End of /NAMES list
```

> El prefijo `@` delante del nick indica operador del canal. El `=` en 353 indica canal público.

#### Errores posibles

| Código | Nombre | Cuándo |
|--------|--------|--------|
| 451 | ERR_NOTREGISTERED | Cliente no ha hecho PASS+NICK+USER |
| 461 | ERR_NEEDMOREPARAMS | `JOIN` sin argumentos |
| 403 | ERR_NOSUCHCHANNEL | Nombre de canal inválido (sin `#`, vacío, demasiado largo) |
| 471 | ERR_CHANNELISFULL | Canal tiene límite `+l` y está lleno *(Fase 6)* |
| 473 | ERR_INVITEONLYCHAN | Canal es `+i` y cliente no está invitado *(Fase 6)* |
| 475 | ERR_BADCHANNELKEY | Canal tiene `+k` y la clave es incorrecta *(Fase 6)* |

#### Edge cases críticos

- `JOIN 0` → el cliente abandona **todos** los canales en los que esté. Hay que iterar `_channels` y llamar a la lógica de PART para cada uno.
- `JOIN #canal,#otro` → algunos clientes envían múltiples canales separados por coma en un solo JOIN. Hay que splitear por `,` y procesar cada uno.
- El cliente intenta hacer JOIN a un canal en el que ya está → **ignorar silenciosamente** (no enviar error, no reenviar respuestas).
- Nombre de canal con caracteres inválidos como espacios o `\a` (bell) → `ERR_NOSUCHCHANNEL`.
- El primer miembro de un canal creado nuevo **siempre** es operador.

#### Números de reply a añadir en `NumericReplies.hpp`
```cpp
#define RPL_TOPIC           332
#define RPL_NOTOPIC         331
#define ERR_NOSUCHCHANNEL   403
#define ERR_CHANNELISFULL   471
#define ERR_INVITEONLYCHAN  473
#define ERR_BADCHANNELKEY   475
// Ya existentes que también se usan:
// RPL_NICKLIST 353, RPL_ENDOFNAMES 366, ERR_NEEDMOREPARAMS 461, ERR_NOTREGISTERED 451
```

---

### 2. `PRIVMSG` — Mensajes privados y de canal

#### Sintaxis
```
PRIVMSG #canal :Mensaje para todos
PRIVMSG nick :Mensaje privado a una persona
```

#### Flujo de ejecución
1. Verificar que el cliente está registrado. Si no → `ERR_NOTREGISTERED (451)`.
2. Verificar que hay al menos 2 parámetros (destinatario y mensaje). Si falta destinatario → `ERR_NORECIPIENT (411)`. Si falta mensaje → `ERR_NOTEXTTOSEND (412)`.
3. Examinar el primer carácter del destinatario:
   - **Empieza por `#`** → es un canal:
     - ¿Existe el canal? Si no → `ERR_NOSUCHCHANNEL (403)`.
     - ¿Es el cliente miembro del canal? Si no → `ERR_CANNOTSENDTOCHAN (404)`.
     - Broadcast a todos los miembros del canal **excepto** al remitente.
   - **No empieza por `#`** → es un nick:
     - ¿Existe ese nick en el servidor? Si no → `ERR_NOSUCHNICK (401)`.
     - Enviar el mensaje al cliente con ese nick.
4. En caso de éxito, **no se envía ninguna respuesta numérica al remitente**. Solo el destinatario (o el canal) recibe el mensaje.

#### Formato del mensaje enviado a destinatarios
```
:<nick_remitente>!<user>@<ip> PRIVMSG <destino> :Texto del mensaje
```

#### Errores posibles

| Código | Nombre | Cuándo |
|--------|--------|--------|
| 451 | ERR_NOTREGISTERED | Cliente no registrado |
| 411 | ERR_NORECIPIENT | Sin destinatario: `PRIVMSG :hola` |
| 412 | ERR_NOTEXTTOSEND | Sin mensaje: `PRIVMSG #canal` |
| 401 | ERR_NOSUCHNICK | El nick destino no existe |
| 403 | ERR_NOSUCHCHANNEL | El canal destino no existe |
| 404 | ERR_CANNOTSENDTOCHAN | Cliente no está en el canal |

#### Edge cases críticos

- `PRIVMSG #canal` sin texto trailing → `ERR_NOTEXTTOSEND`. Cuidado: el parser de `utils.cpp` pone el trailing como último parámetro; si solo hay un param, es el destinatario y falta el mensaje.
- `PRIVMSG` sin ningún parámetro → `ERR_NORECIPIENT`.
- Mensaje a un nick con mayúsculas/minúsculas distintas: IRC es **case-insensitive** para nicks. Podéis decidir si lo implementáis así o no (muchos proyectos 42 no lo hacen).
- El remitente se envía un PRIVMSG a sí mismo → es válido según RFC. Él mismo recibirá su propio mensaje (útil para testing).
- PRIVMSG a un canal sin estar en él → `ERR_CANNOTSENDTOCHAN`. Este es un error común de olvidar.

#### Números de reply a añadir en `NumericReplies.hpp`
```cpp
#define ERR_NOSUCHNICK      401
#define ERR_CANNOTSENDTOCHAN 404
#define ERR_NORECIPIENT     411
#define ERR_NOTEXTTOSEND    412
```

---

### 3. `PART` — Salir de un canal

#### Sintaxis
```
PART #canal
PART #canal :Razón de salida
```

#### Flujo y respuestas
1. Verificar registro → `ERR_NOTREGISTERED`.
2. Verificar parámetros → `ERR_NEEDMOREPARAMS`.
3. ¿Existe el canal? → `ERR_NOSUCHCHANNEL`.
4. ¿Está el cliente en el canal? → `ERR_NOTONCHANNEL (442)`.
5. Notificar a todos los miembros del canal (incluido el que sale):
   ```
   :<nick>!<user>@<ip> PART #canal :Razón
   ```
6. Eliminar al cliente de `_members` del canal.
7. Si el canal queda vacío → eliminar el canal de `_channels` del Server.

#### Números a añadir
```cpp
#define ERR_NOTONCHANNEL    442
```

---

### 4. `QUIT` — Desconexión global

#### Sintaxis
```
QUIT
QUIT :Razón de salida
```

#### Flujo
1. Para cada canal en `_channels` donde el cliente sea miembro:
   - Notificar a todos los otros miembros:
     ```
     :<nick>!<user>@<ip> QUIT :Razón
     ```
   - Eliminar al cliente del canal.
   - Si el canal queda vacío → eliminar el canal.
2. Llamar a `server->disconnectClient(client->getFd())`.

> **Importante:** No llamar a `disconnectClient` dentro del bucle sobre los canales porque invalida iteradores. Primero limpiar todos los canales, luego desconectar.

---

## Parte II — Nueva clase `Channel`

### `Channel.hpp`
```cpp
#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <map>
#include <vector>
#include "Client.hpp"  // solo el puntero, no copia

class Channel
{
private:
    std::string             _name;
    std::string             _topic;
    std::map<int, Client*>  _members;    // fd → Client*
    std::vector<int>        _operators;  // fds de operadores

public:
    Channel();
    Channel(const std::string &name);
    Channel(const Channel &other);
    Channel &operator=(const Channel &other);
    ~Channel();

    // Getters
    const std::string &getName() const;
    const std::string &getTopic() const;
    const std::map<int, Client*> &getMembers() const;

    // Setters
    void setTopic(const std::string &topic);

    // Miembros
    void addClient(Client *client);
    void removeClient(int fd);
    bool isMember(int fd) const;
    bool isOperator(int fd) const;
    void addOperator(int fd);
    void removeOperator(int fd);
    bool isEmpty() const;

    // Mensajería
    // Envía a todos los miembros excepto al sender_fd
    void broadcast(const std::string &message, int sender_fd, Server *server);
    // Envía a TODOS incluido sender_fd
    void broadcastAll(const std::string &message, Server *server);

    // Genera la lista de nicks para RPL_NAMREPLY
    std::string getNickList() const;
};

#endif // CHANNEL_HPP
```

### Notas de implementación de `Channel`

- `_members` usa `Client*` para referenciar los clientes que viven en `Server::clients_`. **No se copia el Client**, solo un puntero. Esto es correcto en C++98.
- `broadcast()` itera `_members` y llama a `server->queueClientData(*it->second, message)` para cada miembro excepto `sender_fd`.
- `getNickList()` itera `_members` y genera un string del tipo `"@operador1 user2 user3"`, añadiendo `@` si el fd está en `_operators`.
- Al hacer `removeClient`, también hay que llamar a `removeOperator` por si era operador.

---

### Cambios en `Server.hpp`

```cpp
#include "Channel.hpp"

// Nuevo atributo privado:
std::map<std::string, Channel> _channels_;

// Nuevos métodos públicos:
void handleJoin(Client &client, const std::vector<std::string> &params);
void handlePrivmsg(Client &client, const std::vector<std::string> &params);
void handlePart(Client &client, const std::vector<std::string> &params);
void handleQuit(Client &client, const std::vector<std::string> &params);

// Helpers útiles:
Client *findClientByNick(const std::string &nick);
```

### Cambios en `NumericReplies.hpp`
Añadir todos los códigos mencionados en las secciones anteriores más:
```cpp
#define RPL_TOPIC           332
#define RPL_NOTOPIC         331
#define ERR_NOSUCHNICK      401
#define ERR_NOSUCHCHANNEL   403
#define ERR_CANNOTSENDTOCHAN 404
#define ERR_NORECIPIENT     411
#define ERR_NOTEXTTOSEND    412
#define ERR_NOTONCHANNEL    442
#define ERR_CHANNELISFULL   471
#define ERR_INVITEONLYCHAN  473
#define ERR_BADCHANNELKEY   475
```

### Cambios en `CommandFactory.cpp`
```cpp
#include "Command/JoinCommand.hpp"
#include "Command/PrivmsgCommand.hpp"
#include "Command/PartCommand.hpp"
#include "Command/QuitCommand.hpp"

// En el constructor:
creators_["JOIN"]    = &JoinCommand::create;
creators_["PRIVMSG"] = &PrivmsgCommand::create;
creators_["PART"]    = &PartCommand::create;
creators_["QUIT"]    = &QuitCommand::create;
```

### Cambios en `Makefile`
```makefile
Command/JoinCommand.cpp \
Command/PrivmsgCommand.cpp \
Command/PartCommand.cpp \
Command/QuitCommand.cpp \
Channel.cpp \
```

### Guard de autenticación en `receiveClientData` (o en cada comando)
Añadir en cada nuevo comando al inicio de `execute()`:
```cpp
if (!client->isAuth()) {
    // enviar ERR_NOTREGISTERED y return
}
```

---

## Parte III — Reparto de tareas (3 personas)

La división está pensada para que cada persona tenga una unidad coherente que pueda implementar, probar y hacer PR de forma independiente.

---

### 👤 Persona A — Infraestructura: Clase `Channel` + `PART` + `QUIT`

**Responsabilidad:** Crea la base que todos necesitan. Las personas B y C dependen de que Channel esté listo.

**Tareas:**

1. Crear `inc/Channel.hpp` con todos los atributos y métodos descritos en la Parte II.
2. Crear `src/Channel.cpp` con la implementación completa:
   - `addClient`, `removeClient`, `isMember`, `isOperator`, `addOperator`, `removeOperator`, `isEmpty`
   - `broadcast()` y `broadcastAll()` usando `server->queueClientData()`
   - `getNickList()` con prefijo `@` para operadores
3. Añadir `std::map<std::string, Channel> _channels_` a `Server.hpp` y declarar `findClientByNick()`.
4. Implementar `findClientByNick()` en `Server.cpp`: iterar `clients_` buscando por nick.
5. Añadir todos los nuevos códigos numéricos a `NumericReplies.hpp`.
6. Crear `src/Command/PartCommand.hpp` y `src/Command/PartCommand.cpp`:
   - Seguir el patrón de `NickCommand` (constructor, `execute`, `create`)
   - Lógica de PART: verificar registro, verificar que existe el canal y el cliente está en él, broadcast PART a todos, `channel.removeClient()`, eliminar canal si está vacío.
7. Crear `src/Command/QuitCommand.hpp` y `src/Command/QuitCommand.cpp`:
   - Iterar todos los canales, notificar y eliminar al cliente de cada uno, luego `disconnectClient`.
8. Registrar PART y QUIT en `CommandFactory.cpp` y en el `Makefile`.
9. Modificar `disconnectClient()` en `Server.cpp` para que también llame a la lógica de QUIT (limpiar canales) cuando un cliente se desconecta de forma inesperada (recv = 0 o -1).

**Archivos a crear/modificar:**
- `inc/Channel.hpp` ✦ nuevo
- `src/Channel.cpp` ✦ nuevo
- `inc/Command/PartCommand.hpp` ✦ nuevo
- `src/Command/PartCommand.cpp` ✦ nuevo
- `inc/Command/QuitCommand.hpp` ✦ nuevo
- `src/Command/QuitCommand.cpp` ✦ nuevo
- `inc/NumericReplies.hpp` — añadir códigos
- `inc/Server.hpp` — añadir `_channels_`, `findClientByNick`, declaraciones de handle*
- `src/Server.cpp` — añadir `findClientByNick`, modificar `disconnectClient`
- `src/Command/CommandFactory.cpp` — registrar PART y QUIT
- `Makefile` — añadir nuevos .cpp

---

### 👤 Persona B — Comando `JOIN`

**Responsabilidad:** Implementar el flujo completo de JOIN, incluyendo la creación de canales y todas las respuestas numéricas al nuevo miembro y a los miembros existentes.

**Prerequisito:** Necesita que `Channel.hpp/cpp` de la Persona A esté disponible (al menos el `.hpp` para poder compilar).

**Tareas:**

1. Crear `inc/Command/JoinCommand.hpp` siguiendo el patrón de `NickCommand.hpp`.
2. Crear `src/Command/JoinCommand.cpp` con `execute()`:

   a. Guard de autenticación: `if (!client->isAuth())` → `ERR_NOTREGISTERED`.
   
   b. Guard de parámetros: `if (params_.empty())` → `ERR_NEEDMOREPARAMS`.
   
   c. Manejar `JOIN 0` (salida de todos los canales): detectar si `params_[0] == "0"` y ejecutar PART implícito en cada canal donde esté el cliente.
   
   d. Splitear `params_[0]` por coma (para `JOIN #a,#b`). Para cada nombre de canal:
      - Validar nombre (empieza por `#`, longitud, sin espacios) → `ERR_NOSUCHCHANNEL`.
      - Verificar si el cliente ya está en el canal → skip silencioso.
      - Buscar canal en `_channels_` del Server.
      - Si no existe → crear canal (`Channel(name)`), insertar en `_channels_`, añadir cliente como miembro Y como operador.
      - Si existe → `channel.addClient(client)`.
      - Construir y enviar las respuestas en orden:
        1. Broadcast a todos (incluido el nuevo): `:<nick>!<user>@<ip> JOIN #canal\r\n`
        2. Enviar RPL_TOPIC o RPL_NOTOPIC solo al nuevo miembro.
        3. Enviar RPL_NAMREPLY (353) al nuevo miembro con `channel.getNickList()`.
        4. Enviar RPL_ENDOFNAMES (366) al nuevo miembro.

3. Implementar `handleJoin()` en `Server.cpp` (o hacerlo todo dentro del command, como el resto de commands del proyecto).

4. Registrar JOIN en `CommandFactory.cpp` y en el `Makefile`.

5. **Testing manual:** Con `nc -C localhost 6667`, hacer `PASS x\r\nNICK test\r\nUSER t 0 * :r\r\nJOIN #general\r\n` y verificar que se reciben los numerics correctos.

**Archivos a crear/modificar:**
- `inc/Command/JoinCommand.hpp` ✦ nuevo
- `src/Command/JoinCommand.cpp` ✦ nuevo
- `src/Command/CommandFactory.cpp` — registrar JOIN
- `Makefile` — añadir JoinCommand.cpp

---

### 👤 Persona C — Comando `PRIVMSG` + integración de guards

**Responsabilidad:** Implementar PRIVMSG (la funcionalidad de mensajería, la más visible) y asegurarse de que los comandos existentes (NICK, USER, etc.) rechacen correctamente a clientes no registrados.

**Prerequisito:** Necesita Channel.hpp de la Persona A para el broadcast de canal.

**Tareas:**

1. Añadir guard de autenticación a los comandos existentes que actualmente no lo tienen. Revisar `NickCommand`, `UserCommand`, etc. y asegurarse de que si se llama a un comando post-registro antes de estar autenticado, responden con `ERR_NOTREGISTERED`. (Actualmente algunos comandos no tienen este check).

2. Crear `inc/Command/PrivmsgCommand.hpp`.

3. Crear `src/Command/PrivmsgCommand.cpp` con `execute()`:

   a. Guard de autenticación → `ERR_NOTREGISTERED`.
   
   b. Sin parámetros → `ERR_NORECIPIENT`.
   
   c. Solo 1 parámetro (falta el mensaje) → `ERR_NOTEXTTOSEND`.
   
   d. Obtener destinatario (`params_[0]`) y texto (`params_[1]`).
   
   e. Si destinatario empieza por `#`:
      - Buscar canal en `_channels_`. Si no existe → `ERR_NOSUCHCHANNEL`.
      - Verificar que cliente es miembro. Si no → `ERR_CANNOTSENDTOCHAN`.
      - Construir mensaje: `:<nick>!<user>@<ip> PRIVMSG #canal :texto\r\n`
      - Llamar a `channel.broadcast(message, client->getFd(), server)`.
   
   f. Si destinatario NO empieza por `#`:
      - Llamar a `server->findClientByNick(destinatario)`. Si null → `ERR_NOSUCHNICK`.
      - Construir mensaje: `:<nick>!<user>@<ip> PRIVMSG <nick_dest> :texto\r\n`
      - Llamar a `server->queueClientData(*destClient, message)`.
   
   g. En caso de éxito, **no enviar ninguna respuesta al remitente**.

4. Registrar PRIVMSG en `CommandFactory.cpp` y en el `Makefile`.

5. Implementar `findClientByNick()` en `Server.cpp` si la Persona A no lo ha hecho todavía (coordinad).

6. **Testing manual:** Conectar dos clientes con `nc`, hacer que ambos hagan JOIN al mismo canal, y verificar que los mensajes llegan correctamente al otro.

**Archivos a crear/modificar:**
- `inc/Command/PrivmsgCommand.hpp` ✦ nuevo
- `src/Command/PrivmsgCommand.cpp` ✦ nuevo
- `src/Command/CommandFactory.cpp` — registrar PRIVMSG
- `Makefile` — añadir PrivmsgCommand.cpp
- `src/Command/NickCommand.cpp` — revisar guards
- `src/Command/UserCommand.cpp` — revisar guards

---

## Parte IV — Orden de trabajo y coordinación

```
Día 1
├── Persona A: Channel.hpp lista y Channel.cpp esqueleto → commit en rama feature/channel
├── Persona B: JoinCommand.hpp + execute() sin broadcast (solo lógica de canal)
└── Persona C: PrivmsgCommand.hpp + guards en commands existentes

Día 2
├── Persona A: Channel.cpp completo + PartCommand + QuitCommand
├── Persona B: Broadcast en JOIN + RPL_NAMREPLY + testing
└── Persona C: PRIVMSG completo (canal y nick) + testing cruzado con B

Día 3 (integración)
└── Los tres: merge de ramas, testing con cliente IRC real (irssi/weechat)
    Probar: JOIN, PRIVMSG, PART, QUIT, edge cases
```

### Dependencias entre personas

```
Persona A (Channel)
    ↓ Channel.hpp necesario para:
Persona B (JOIN) ──── necesita addClient, broadcast, getNickList, addOperator
Persona C (PRIVMSG) ── necesita broadcast, isMember, findClientByNick (Server)
```

**Punto de sincronización clave:** La Persona A debe tener al menos el `.hpp` de `Channel` completo y `Server.hpp` actualizado antes de que B y C puedan compilar. Lo más práctico es que A haga un commit temprano solo con los headers.

---

## Parte V — Checklist de testing

### Test con `nc`
```bash
# Terminal 1 — cliente 1
nc -C 127.0.0.1 6667
PASS tupassword
NICK alice
USER alice 0 * :Alice Real
JOIN #test

# Terminal 2 — cliente 2
nc -C 127.0.0.1 6667
PASS tupassword
NICK bob
USER bob 0 * :Bob Real
JOIN #test
PRIVMSG #test :Hola canal
PRIVMSG alice :Hola privado
```

### Qué verificar
- [ ] Alice recibe la lista de nombres al hacer JOIN (353 + 366)
- [ ] Bob recibe notificación de JOIN de alice cuando alice ya está en el canal
- [ ] El mensaje de Bob al canal le llega a Alice pero NO a Bob
- [ ] El privado de Bob a Alice le llega a Alice
- [ ] PART notifica a todos en el canal
- [ ] QUIT elimina al cliente de todos los canales y notifica
- [ ] `JOIN 0` saca al cliente de todos los canales
- [ ] PRIVMSG a canal sin estar en él → 404
- [ ] PRIVMSG a nick inexistente → 401
- [ ] JOIN a canal con nombre inválido → 403
- [ ] Comandos sin estar registrado → 451
