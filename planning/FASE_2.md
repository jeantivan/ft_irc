# Fase 2: Redes y Core del Servidor

- Manejar argumentos de entrada: Configurar el ejecutable para recibir obligatoriamente el formato ./ircserv <port> <password>

- Inicializar Sockets: Implementar la creación del socket del servidor para establecer comunicación vía TCP/IP (v4 o v6).

- Configurar I/O no bloqueante: Usar fcntl(fd, F_SETFL, O_NONBLOCK) (es la única bandera permitida de fcntl(), especialmente crucial si trabajas en MacOS).


- Implementar el bucle principal: Configurar la función poll() (o equivalentes permitidos como select, kqueue, epoll) para poder manejar múltiples clientes simultáneamente sin que el servidor se cuelgue.
