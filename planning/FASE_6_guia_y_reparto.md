# Fase 6 — Guía y Reparto de Tareas (Privilegios y Comandos de Operador)

## 0. Punto de partida

Según vuestro código actual:

- `Channel` ya tiene `operators_` (`std::set<int>`), `isOperator(fd)`, `addOperator(fd)`, `removeOperator(fd)` implementados.
- **Faltan** en `Channel`: los atributos de modo (`_inviteOnly`, `_topicRestricted`, `_password`, `_userLimit`, `_invitedUsers`) y sus getters/setters.
- `JoinCommand::execute()` existe pero **no** comprueba todavía password, invite-only ni límite de usuarios (retrofit pendiente).
- No existen aún: `KickCommand`, `InviteCommand`, `TopicCommand`, `ModeCommand` (ni sus `.hpp`/`.cpp`, siguiendo vuestra convención en `src/Command/` e `inc/Command/`).
- `NumericReplies.hpp` no tiene todavía los códigos que necesitáis para esta fase (ver más abajo, cada persona debe añadir los suyos).

**Regla de dependencia importante:** el comando `MODE` y el retrofit de `JOIN` necesitan que `Channel` tenga los nuevos atributos de modo. Por eso la Persona A debe entregar esa parte de `Channel` **primero** (o al menos la interfaz: firmas de métodos en el `.hpp`) para que B y C puedan compilar y trabajar en paralelo sin bloquearse.

---

## 1. Orden de trabajo recomendado

1. **Persona A** define y sube primero el `.hpp` de `Channel` ampliado (solo firmas, aunque los `.cpp` los implemente después) → esto desbloquea a B y C.
2. **A, B y C trabajan en paralelo** en sus comandos respectivos.
3. Integración conjunta: registrar los 4 comandos nuevos en `CommandFactory`, probar con el cliente de referencia, revisar el checklist manual conjunto (sección 5).

---

## 2. Persona A — Arquitectura de modos + comando MODE

Esta es la parte más grande porque `MODE` es "el jefe final", así que solo lleva esto (nada más).

### 2.1 Subtarea 6.1 — Arquitectura de Modos de Canal

En `Channel.hpp` / `Channel.cpp`, añadir:

```cpp
bool         _inviteOnly;      // modo i
bool         _topicRestricted; // modo t
std::string  _password;        // modo k (vacío = sin clave)
unsigned int _userLimit;       // modo l (0 = sin límite)
std::vector<int> _invitedUsers; // fds invitados si _inviteOnly
```

Métodos a añadir:
- `bool isInviteOnly() const;` / `void setInviteOnly(bool state);`
- `bool isTopicRestricted() const;` / `void setTopicRestricted(bool state);`
- `const std::string &getPassword() const;` / `void setPassword(const std::string &key);`
- `unsigned int getUserLimit() const;` / `void setUserLimit(unsigned int limit);`
- `bool isInvited(int fd) const;` / `void addInvited(int fd);` / `void removeInvited(int fd);` (opcional, limpiar tras el JOIN)

Recordad inicializar estos valores en el constructor de `Channel` (`false`, `""`, `0`, vector vacío).

### 2.2 Subtarea 6.5 — Comando MODE

Crear `Command/ModeCommand.hpp` / `.cpp` siguiendo el mismo patrón que `JoinCommand`/`PassCommand` (herencia de `Command`, `create()` estático para el `CommandFactory`).

**Sintaxis a soportar:** `MODE <canal> <+/-flags> [parámetros]`
Ejemplos reales: `MODE #canal +i`, `MODE #canal +k clave`, `MODE #canal +l 10`, `MODE #canal +o nick`, `MODE #canal -o nick`, y combinaciones como `MODE #canal +tk clave`.

**Validaciones en orden:**
1. ¿Canal existe? → si no, `ERR_NOSUCHCHANNEL` (ya la tenéis definida).
2. ¿Cliente está en el canal? → si no, `ERR_NOTONCHANNEL`.
3. ¿Cliente es operador? → si no, `ERR_CHANOPRIVSNEEDED` (**añadir este código a `NumericReplies.hpp`: 482**).
4. Parsear la cadena de flags carácter a carácter, llevando un booleano de "añadir" (`+`) o "quitar" (`-`) que cambia según se van encontrando los signos.
5. Por cada flag, consumir el parámetro correspondiente si lo necesita (`k`, `l`, `o` consumen un parámetro; `i`, `t` no).
6. Aplicar el cambio llamando a los setters de `Channel` que creasteis en 2.1.
7. Construir el mensaje de confirmación y hacer `broadcastAll()` a todo el canal, tipo:
   `:nick!user@host MODE #canal +o target_nick`

**Casos límite a cubrir:**
- `MODE #canal` sin flags → responder con el modo actual del canal (`RPL_CHANNELMODEIS`, código **324**, añadidlo también).
- `-k` no necesita el parámetro de la clave (la IRC real lo acepta igual, pero podéis simplificar exigiéndolo o no; documentadlo en el README).
- `+o`/`-o` con un nick que no está en el canal → `ERR_USERNOTINCHANNEL` (**441**, añadidlo).
- Si falta el parámetro obligatorio de una flag → `ERR_NEEDMOREPARAMS` (ya existe).

### 2.3 Entregable de A
- `Channel.hpp`/`.cpp` con los nuevos atributos y métodos.
- `ModeCommand.hpp`/`.cpp`.
- Nuevos códigos en `NumericReplies.hpp`: `ERR_CHANOPRIVSNEEDED (482)`, `RPL_CHANNELMODEIS (324)`, `ERR_USERNOTINCHANNEL (441)`.

---

## 3. Persona B — Comandos KICK e INVITE

### 3.1 Subtarea 6.2 — Comando KICK

Crear `Command/KickCommand.hpp` / `.cpp`.

**Sintaxis:** `KICK <canal> <nick> [:razón]`

**Validaciones en este orden estricto (tal como pide el enunciado):**
1. ¿Canal existe? → `ERR_NOSUCHCHANNEL`.
2. ¿Quien ejecuta el comando está en el canal? → `ERR_NOTONCHANNEL`.
3. ¿Quien ejecuta es operador? → `canal.isOperator(client->getFd())`. Si no → `ERR_CHANOPRIVSNEEDED` (código que añade la Persona A; coordinad para no duplicarlo en `NumericReplies.hpp`).
4. ¿El usuario objetivo está en el canal? → si no, `ERR_USERNOTINCHANNEL`.
5. Si todo OK: `canal.removeClient(target_fd)` y `broadcastAll()` del mensaje de expulsión, incluyendo al expulsado:
   `:nick!user@host KICK #canal target_nick :razón`

**Detalle importante:** el expulsado debe recibir el mensaje KICK *antes* de que se le quite de `members_`, si no, `broadcastAll` no le llegará. Revisad el orden en vuestro `Channel::broadcastAll` vs `removeClient` (mirad cómo lo resolvisteis en `QuitCommand` como referencia, ahí hacéis `broadcastAll` y luego `removeClient`).

### 3.2 Subtarea 6.3 — Comando INVITE

Crear `Command/InviteCommand.hpp` / `.cpp`.

**Sintaxis:** `INVITE <nick> <canal>`

**Validaciones:**
1. ¿El nick objetivo existe? (`server->findClientByNick()`, ya existe) → si no, `ERR_NOSUCHNICK` (ya existe).
2. ¿El canal existe? → `ERR_NOSUCHCHANNEL`.
3. ¿Quien invita está en el canal? → `ERR_NOTONCHANNEL`.
4. Si el canal es `+i` (invite-only): solo operadores pueden invitar → si no es operador, `ERR_CHANOPRIVSNEEDED`.
5. ¿El objetivo ya está en el canal? → `ERR_USERONCHANNEL` (**443, añadidlo a `NumericReplies.hpp`**).
6. Si todo OK: añadir el fd del invitado a `_invitedUsers` (método de la Persona A, coordinad el nombre exacto: `addInvited(fd)`), enviar `RPL_INVITING` (**341, añadidlo**) al que invita, y enviar el mensaje `INVITE` al invitado:
   `:nick!user@host INVITE target_nick #canal`

### 3.3 Entregable de B
- `KickCommand.hpp`/`.cpp`.
- `InviteCommand.hpp`/`.cpp`.
- Nuevos códigos: `ERR_USERONCHANNEL (443)`, `RPL_INVITING (341)`.

---

## 4. Persona C — Comando TOPIC + retrofit de JOIN + checklist de pruebas

### 4.1 Subtarea 6.4 — Comando TOPIC

Crear `Command/TopicCommand.hpp` / `.cpp`.

**Sintaxis:** `TOPIC <canal> [:nuevo topic]`

**Lógica:**
1. ¿Canal existe? → `ERR_NOSUCHCHANNEL`.
2. ¿Cliente está en el canal? → `ERR_NOTONCHANNEL`.
3. Si **no** se pasa el segundo parámetro (solo se quiere ver el topic):
   - Si el canal no tiene topic → `RPL_NOTOPIC` (ya existe, 331).
   - Si tiene topic → `RPL_TOPIC` (ya existe, 332).
4. Si **sí** se pasa un nuevo topic (se quiere cambiar):
   - Si el canal es `+t` (`isTopicRestricted()`) y el cliente **no** es operador → `ERR_CHANOPRIVSNEEDED`.
   - Si no, aplicar `canal.setTopic(nuevo_topic)` y hacer `broadcastAll()`:
     `:nick!user@host TOPIC #canal :nuevo topic`

> Nota del apunte de Antonio en el FASE_6.md: gestionar `RPL_TOPICWHOTIME` en JOIN. Este código (**333**) indica quién y cuándo cambió el topic. Si queréis implementarlo bien, guardad en `Channel` quién puso el topic (`_topicSetBy`) y cuándo (`_topicSetAt`, timestamp `time_t`), y enviad `RPL_TOPICWHOTIME` justo después de `RPL_TOPIC` cuando un cliente hace `JOIN` a un canal con topic. Si no os da tiempo, marcadlo como pendiente en el README, no bloquea el resto.

### 4.2 Retrofit obligatorio de JOIN

Modificar `JoinCommand::execute()` (en `src/Command/JoinCommand.cpp`) para, **antes de meter al cliente en el canal**, comprobar en este orden:

1. **Password (`+k`):** si `canal->getPassword()` no está vacío, comparar con la clave que llega en `JOIN #canal clave` (ya veis que `JoinCommand` parsea `channPasword` como vector, revisadlo). Si no coincide → `ERR_BADCHANNELKEY` (ya existe, 475).
2. **Invite-only (`+i`):** si `canal->isInviteOnly()` es true, comprobar `canal->isInvited(client->getFd())`. Si no está invitado → `ERR_INVITEONLYCHAN` (ya existe, 473).
3. **Límite (`+l`):** si `canal->getUserLimit() > 0`, comprobar que `canal->getMembers().size() < userLimit`. Si está lleno → `ERR_CHANNELISFULL` (ya existe, 471).

Si pasa las 3 comprobaciones, seguir con la lógica actual de `JOIN` (añadir al canal, `namreply`, etc.). Aprovechad para, si implementáis `RPL_TOPICWHOTIME`, enviarlo aquí justo tras el `RPL_TOPIC`/`RPL_NOTOPIC` del JOIN.

**Importante:** si el canal es nuevo (se crea en este mismo JOIN), el creador debe añadirse automáticamente como operador (`canal.addOperator(fd)`) — comprobad que esto ya se hace; si no, añadidlo, porque si no nadie podrá nunca hacer KICK/INVITE/MODE/TOPIC restringido en canales nuevos.

### 4.3 Checklist manual de pruebas (Fase 6)

Con el cliente de referencia (2-3 ventanas / usuarios), probar y marcar:

- [ ] Usuario no-operador intenta `KICK` → recibe `ERR_CHANOPRIVSNEEDED`.
- [ ] Operador expulsa a un usuario del canal → ambos ven el mensaje KICK, el expulsado ya no aparece en `NAMES`.
- [ ] `INVITE` a un canal normal (sin `+i`) funciona igual para cualquier miembro.
- [ ] Canal `+i`: usuario no invitado intenta `JOIN` → `ERR_INVITEONLYCHAN`; tras `INVITE`, puede entrar.
- [ ] `TOPIC` sin argumento en canal sin topic → `RPL_NOTOPIC`; con topic → `RPL_TOPIC`.
- [ ] Canal `+t`: usuario no-operador intenta cambiar el topic → `ERR_CHANOPRIVSNEEDED`; operador sí puede.
- [ ] `MODE #canal +k clave`: un JOIN sin clave o con clave incorrecta → `ERR_BADCHANNELKEY`; con clave correcta entra.
- [ ] `MODE #canal +l 2`: tercer usuario intenta entrar → `ERR_CHANNELISFULL`.
- [ ] `MODE #canal +o nick` da operador; ese usuario ya puede hacer KICK/INVITE/TOPIC restringido/MODE.
- [ ] `MODE #canal -o nick` quita el privilegio correctamente.
- [ ] `MODE #canal` sin flags devuelve el estado actual de los modos.
- [ ] Ningún comando de esta fase crashea el servidor con parámetros incompletos o mal formados (probar sin parámetros, con nicks/canales inexistentes, etc.).

### 4.4 Entregable de C
- `TopicCommand.hpp`/`.cpp`.
- Modificación de `JoinCommand.cpp` (retrofit).
- Checklist de pruebas ejecutado y documentado (puede ir en un `TESTING.md` o sección del README).

---

## 5. Integración final (los 3 juntos)

1. Registrar `KickCommand`, `InviteCommand`, `TopicCommand`, `ModeCommand` en `CommandFactory` (mismo patrón que el resto de comandos).
2. Añadir los 4 nuevos `.cpp` al `Makefile` (`FILES = ...`).
3. Revisar que no haya códigos numéricos duplicados en `NumericReplies.hpp` (A añade 482/324/441, B añade 443/341, C reutiliza los existentes).
4. Pasar juntos el checklist de la sección 4.3.
5. Actualizar el README (sección de IA usada, si aplica) y el `FASE_6.md` marcando lo completado.

---

## 6. Resumen del reparto

| Persona | Tareas | Archivos nuevos/modificados |
|---|---|---|
| **A** | Arquitectura de modos en `Channel` + comando `MODE` | `Channel.hpp/.cpp`, `ModeCommand.hpp/.cpp` |
| **B** | Comandos `KICK` e `INVITE` | `KickCommand.hpp/.cpp`, `InviteCommand.hpp/.cpp` |
| **C** | Comando `TOPIC` + retrofit de `JOIN` + testing manual | `TopicCommand.hpp/.cpp`, `JoinCommand.cpp`, checklist |
