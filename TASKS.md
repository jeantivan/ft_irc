# ircserv — Tareas para pasar la corrección

Lista de trabajo mínima para que el servidor aguante la evaluación (no crashea nunca,
poll antes de cada I/O, reconstruye mensajes partidos). Lo que **no** está aquí se ha
decidido dejar como está por ahora — ver `robustez-backlog` / el artifact para la lista completa
y el detalle ampliado de cada punto:

<https://claude.ai/code/artifact/50b561dc-8d95-4cf9-9bb7-489a8b6fb18a>

## Cómo trabajar

- **Reclama una tarea** escribiendo tu nombre en `Responsable:`.
- Una tarea = una rama = una PR. El título de la PR empieza por el ID: `B5: ignorar SIGPIPE`.
- Respeta el orden de las oleadas: dentro de una oleada las tareas son independientes;
  entre oleadas hay dependencias.
- Copia el bloque **Hecho cuando** a la descripción de la PR y úsalo como checklist.

## Estado

| ID | Tarea | Oleada | Responsable | Estado |
|----|-------|--------|-------------|--------|
| B5 | Ignorar SIGPIPE | 1 | | ⬜ |
| E1 | Flag de señal seguro | 1 | | ⬜ |
| D1 | No leer fuera del buffer de `recv` | 1 | | ⬜ |
| C4 | Índice del bucle con signo | 1 | | ⬜ |
| F1 | Quitar `errno` tras `recv`/`send` | 1 | | ⬜ |
| C1+A1 | Desconexión única + no borrar mientras se itera | 2 | | ⬜ |
| B1 | `accept` que falla no tumba el server | 3 | | ⬜ |
| B3 | Sobrevivir a `bad_alloc` | 3 | | ⬜ |
| D2 | Reconstruir mensajes (framing `\r\n`) | 4 | | ⬜ |
| B6+B7 | Cola de salida + no escribir sin POLLOUT | 5 | | ⬜ |
| F2 | `Server` no copiable | 6 | | ⬜ |
| F5 | README que cumple el subject | 6 | | ⬜ |

---

## Oleada 1 — Quick wins (independientes, ~15 min cada una)

### B5 — Ignorar SIGPIPE

**Responsable:** ________

**Ficheros:** `src/main.cpp`

**Qué falla:** si un cliente cierra la conexión de golpe justo cuando el server le hace
`send()`, el proceso recibe `SIGPIPE` y muere (acción por defecto = terminar).

**Qué hacer (orientativo):**
- En `main()`, antes de crear el `Server`: `signal(SIGPIPE, SIG_IGN);`
- Con eso, `send()` a un socket roto devuelve `-1` en lugar de matar el proceso.
- Asegúrate de que el camino `send() == -1` acaba desconectando al cliente (se cierra en C1/B6).

**Hecho cuando:** cliente A enviando en bucle, cliente B hace Ctrl-C / cierra abrupto → el
server sigue vivo y termina desconectando a B.

---

### E1 — Flag de señal seguro

**Responsable:** ________

**Ficheros:** `inc/Server.hpp`, `src/Server.cpp`

**Qué falla:** `static bool signal_received_` se escribe desde el handler de señal. El
estándar solo garantiza acceso seguro desde un handler a `volatile sig_atomic_t`. Con `-O2`
el compilador puede cachear la lectura en `run()` y no ver nunca el Ctrl-C.

**Qué hacer (orientativo):**
- Cambiar el tipo a `static volatile sig_atomic_t signal_received_;` en el `.hpp` y en la
  definición del `.cpp`.
- El handler solo hace `signal_received_ = 1;` (ya cumple, no toca nada más).
- Bucle de `run()`: `while (signal_received_ == 0)`.

**Hecho cuando:** compilar con `-O2`; Ctrl-C cierra limpio y ejecuta el destructor, 5 veces
seguidas.

---

### D1 — No leer fuera del buffer de `recv`

**Responsable:** ________

**Ficheros:** `src/Server.cpp` (`receiveClientData`, ~líneas 187-223)

**Qué falla:** `recv(fd, buffer, sizeof(buffer), 0)` puede llenar los 1024 bytes sin dejar
un `\0`. Luego `buffer` se usa como cadena C (`message << buffer`, `send(... .c_str())`) y
se lee más allá del array hasta encontrar un cero en la pila → fuga de datos de la pila a
otros clientes, o crash. Se dispara enviando exactamente 1024 bytes.

**Qué hacer (orientativo):**
- Construir un `std::string` con la longitud exacta:
  `std::string data(buffer, bytes_received);`
- Trabajar siempre con `data`, nunca con el `char[]` crudo. Ese mismo `data` se usará en D2.

**Hecho cuando:** `nc` enviando exactamente 1024 bytes sin salto de línea → el server
reenvía esos 1024 bytes exactos, sin basura pegada. Con valgrind/ASan: cero lecturas
inválidas.

---

### C4 — Índice del bucle con signo

**Responsable:** ________

**Ficheros:** `src/Server.cpp:137`

**Qué falla:** `int i = connections_.size() - 1` — `size()` es `size_t` (sin signo). Si
algún día `connections_` queda vacío, `0 - 1` es un número enorme y el bucle se va a
índices basura. Hoy no ocurre (siempre está el listener), pero es una suposición frágil.

**Qué hacer (orientativo):**
- `for (int i = static_cast<int>(connections_.size()) - 1; i >= 0; --i)`
- Si haces C1 y reescribes este bucle, aplica el mismo cuidado en el nuevo.

**Hecho cuando:** revisión de código.

---

### F1 — Quitar `errno` tras `recv` / `send`

**Responsable:** ________

**Ficheros:** `src/Server.cpp:201`, `:229`, `:232`

**Qué falla:** los tres puntos llaman a `std::strerror(errno)` después de `recv`/`send`.
Consultar `errno` tras una operación de lectura/escritura está marcado como falta en la hoja
de corrección de ft_irc.

**Qué hacer (orientativo):**
- Sustituir los mensajes por texto fijo + el fd, p. ej.:
  `std::cerr << "[ircserver]: recv error on client " << fd << std::endl;`
- `grep -rn 'errno\|strerror' src/ inc/` → solo debería quedar (si acaso) en `init()`
  (`getaddrinfo`/`bind`, que no son read/write). Confirmadlo en equipo.

**Hecho cuando:** `grep` limpio de `errno` en los caminos de `recv`/`send`.

---

## Oleada 2 — Cimientos del bucle (una sola persona, van juntas)

### C1 + A1 — Desconexión en un único sitio, y sin borrar mientras se itera

**Responsable:** ________

**Ficheros:** `src/Server.cpp` (`run`, `receiveClientData`), `inc/Server.hpp`

**Qué falla:**
- `run()` recorre `connections_` por índice y, a la vez, `receiveClientData` hace `erase()`
  en ese mismo vector: al desconectar un cliente (línea ~205) y en **cada** `send` fallido
  del reenvío (línea ~231). Los índices se desplazan bajo los pies del bucle externo → se
  acaban procesando `revents` contra el fd equivocado.
- Además, cuando `send()` falla se borra de `connections_` pero **no** de `clients_`: el
  `std::map` se llena de clientes muertos y deja de reflejar `connections_`.

**Qué hacer (orientativo):**
1. Crear un método privado `void disconnectClient(size_t index)`: lee el fd, `close(fd)`,
   `clients_.erase(fd)`, `connections_.erase(connections_.begin() + index)`, un log de una
   línea.
2. Añadir un miembro `std::vector<int> to_disconnect_;`
3. En `receiveClientData`: **nunca** llamar a `erase`. Si `recv <= 0` o `send` falla →
   `to_disconnect_.push_back(fd);` y continúa.
4. En `run()`, **después** del `for` que procesa `revents`: recorrer `to_disconnect_`,
   localizar el índice actual de cada fd, llamar a `disconnectClient(index)`, y vaciar la
   lista.
5. Aceptar nuevas conexiones durante el ciclo está bien: los `pollfd` nuevos tienen
   `revents == 0` y no se miran hasta el siguiente `poll`.

**Hecho cuando:** 5 clientes; uno hace Ctrl-C y otro provoca fallo de envío en el mismo
ciclo → los 3 restantes siguen recibiendo, `clients_.size()` baja de forma correcta, y con
logs de `fd`/`index` no hay cruces.

---

## Oleada 3 — No caerse nunca

### B1 — `accept` que falla no tumba el server

**Responsable:** ________

**Ficheros:** `src/Server.cpp:154-183`

**Qué falla:** `if (new_fd == -1) throw std::runtime_error(...)` — la excepción sube por
`run()` hasta el `catch` de `main` y el proceso termina. Cualquier fallo transitorio
(cliente que cierra antes de tiempo, sin fds libres, señal) mata el servidor.

**Qué hacer (orientativo):**
- Si `new_fd == -1`: log de una línea (sin `errno`) y `return;` — no lanzar.
- (Opcional, barato) si `clients_.size()` supera un máximo razonable (p. ej. 1024), enviar
  `"ERROR :Server full\r\n"` best-effort, `close(new_fd)` y `return`.

**Hecho cuando:** `ulimit -n 64`, saturar de conexiones → un intento de conexión más no
tumba el servidor y los clientes ya conectados siguen operativos.

---

### B3 — Sobrevivir a `bad_alloc`

**Responsable:** ________

**Ficheros:** `src/Server.cpp` (`run`, `acceptNewClient`, `receiveClientData`), `src/main.cpp`

**Qué falla:** `connections_.push_back`, insertar en `clients_`, construir
`std::string`/`std::stringstream`… todos pueden lanzar `std::bad_alloc`. Hoy sube hasta
`main` y el servidor muere. El subject exige aguantar *"incluso al quedarse sin memoria"*.

**Qué hacer (orientativo):**
- En `run()`, envolver el manejo de cada fd:
  ```cpp
  try {
      // accept / receive de este fd
  } catch (const std::exception& e) {
      std::cerr << "[ircserver]: " << e.what() << std::endl;
  }
  ```
  Así un fallo con un cliente no rompe el bucle.
- En `acceptNewClient`: si algo lanza después de `accept()`, `close(new_fd)` y `return`
  (no dejar el fd huérfano).
- `main` mantiene su `try/catch` como última red, solo para errores de arranque.
- Dentro de los `catch` no hagas concatenaciones de `std::string` (pueden volver a lanzar).

**Hecho cuando:** con `setrlimit(RLIMIT_AS, ...)` apretado (o un `operator new` de test que
falla tras N llamadas), conectar / enviar / desconectar en bucle → el servidor loguea
errores pero sigue aceptando y sirviendo.

---

## Oleada 4 — Reconstrucción de mensajes

### D2 — Reconstruir los mensajes (framing por `\r\n`)

**Responsable:** ________

**Ficheros:** `inc/Client.hpp`, `src/Client.cpp`, `src/Server.cpp`

**Qué falla:** cada trozo que devuelve `recv` se procesa/reenvía tal cual. TCP puede partir
un comando en varios `recv` o pegar varios en uno. El subject lo prueba explícitamente
(`nc -C` + Ctrl-D en varias partes). Sin esto, ningún cliente IRC real funcionará.

**Qué hacer (orientativo):**
1. Añadir a `Client`: `std::string in_buffer_;` con métodos
   `void appendData(const std::string&)` y `bool nextLine(std::string& out)`.
2. Tras `recv`: `client.appendData(data);` (el `data` dimensionado de D1).
3. En bucle: mientras haya `\r\n` en el buffer (acepta también `\n` a secas por tolerancia),
   extraer la línea sin el terminador, procesarla / reenviarla, y borrarla del buffer.
4. Lo que quede sin terminador se conserva para el siguiente `recv`.
5. Tope simple: si `in_buffer_` supera ~512 bytes sin `\r\n`, truncar o descartar esa línea
   (decidid cuál) para que no crezca sin fin.

**Hecho cuando:** `nc -C 127.0.0.1 <port>`, escribir `com` Ctrl-D `man` Ctrl-D `d` Enter →
el servidor procesa **un** único mensaje `command`. Enviar dos líneas pegadas → procesa dos.

---

## Oleada 5 — Escritura no bloqueante

### B6 + B7 — Cola de salida por cliente y no escribir sin POLLOUT

**Responsable:** ________

**Ficheros:** `inc/Client.hpp`, `src/Server.cpp`

**Qué falla:**
- El reenvío hace `send()` a cada cliente sin que `poll` haya reportado que ese fd es
  escribible. Regla del subject: *"si escribes en un fd sin poll, tu nota será 0"*.
- Un `send()` no bloqueante puede escribir menos bytes de los pedidos o devolver `-1` con
  `EAGAIN` si el buffer del kernel está lleno. Hoy eso desconecta a un cliente sano o trunca
  el mensaje.

**Qué hacer (orientativo):**
1. Añadir a `Client`: `std::string out_buffer_;`
2. Reenviar = `client.out_buffer_ += mensaje;` — nunca `send` directo en el reenvío.
3. Gestionar POLLOUT: si `out_buffer_` no está vacío →
   `connections_[i].events = POLLIN | POLLOUT;`  al vaciarse → volver a solo `POLLIN`.
4. En `run()`, si `revents & POLLOUT`:
   ```cpp
   int n = send(fd, out_buffer_.c_str(), out_buffer_.size(), 0);
   if (n > 0) out_buffer_.erase(0, n);
   else if (n == -1) to_disconnect_.push_back(fd);   // sin mirar errno
   ```
5. Tope: si `out_buffer_` supera ~64 KB, marcar a ese cliente para desconexión (lee
   demasiado lento).

**Hecho cuando:** con `SO_SNDBUF` pequeño y un cliente que lee despacio → los mensajes
llegan completos y en orden, sin desconexiones; y al revisar el código no queda ningún
`send`/`recv` fuera de una rama de `revents` (el `send` del destructor puede quedarse,
comentado como best-effort de apagado).

---

## Oleada 6 — Cierre

### F2 — `Server` no copiable

**Responsable:** ________

**Ficheros:** `inc/Server.hpp`, `src/Server.cpp:27-39`

**Qué falla:** el constructor de copia y `operator=` copian `listener_` (el fd). Si alguien
copiara un `Server` (devolverlo por valor, meterlo en un contenedor), dos destructores
harían `close()` del mismo fd. Hoy están en `private` pero **con cuerpo**.

**Qué hacer (orientativo):**
- Dejar solo la **declaración** `private` en el `.hpp` (ya está) y **borrar las
  definiciones** del `.cpp`. Si alguien los usa, falla el enlazado (que es lo que queremos).
- Comentario: `// Server no es copiable: posee descriptores y buffers`

**Hecho cuando:** el `.cpp` ya no define esas dos funciones y el proyecto compila y enlaza.

---

### F5 — README que cumple el subject

**Responsable:** ________

**Ficheros:** `README.md`

**Qué falla:** tiene 9 bytes. El capítulo V del subject exige un formato concreto; sin él la
entrega está incompleta.

**Qué hacer (orientativo):**
- Primera línea **en cursiva**:
  `*This project has been created as part of the 42 curriculum by <login1>, <login2>, ...*`
- `## Description`: qué es ft_irc, objetivo y un resumen breve.
- `## Instructions`: `make`, `./ircserv <port> <password>`, y el cliente de referencia
  elegido (p. ej. irssi o HexChat).
- `## Resources`: Beej's Guide to Network Programming, RFC 1459 / 2812, Modern IRC
  (modern.ircdocs.horse), y un párrafo **"AI usage"** explicando para qué tareas y qué
  partes del proyecto se usó IA.
- Todo en inglés.

**Hecho cuando:** el `README.md` tiene la primera línea en cursiva con los logins y las
secciones Description / Instructions / Resources.
