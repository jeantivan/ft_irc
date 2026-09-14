# IRC Server Tests

Estos tests automatizados están diseñados para probar la funcionalidad básica de tu servidor `ft_irc` de acuerdo con los requisitos del PDF (RFCs de IRC).
Se han colocado fuera del directorio `repo` para no interferir ni modificar tu proyecto original.

## Requisitos

- `python3`

## Archivos

- `test_irc.py`: Un script simple en Python que usa sockets para enviar comandos de texto crudo (RAW) a tu servidor y verifica que responda adecuadamente. (Pruebas básicas).
- `test_advanced.py`: Un script más complejo que prueba múltiples clientes a la vez, manejo concurrente y todos los casos límite y modos requeridos.

## Cómo usar

1. Compila y ejecuta tu servidor en una terminal:
   ```bash
   cd ../repo
   make
   ./ircserv 6667 mypassword
   ```

2. En otra terminal, ejecuta los scripts de prueba pasándoles el mismo puerto y contraseña que usaste en el servidor:
   ```bash
   python3 test_irc.py 6667 mypassword

   # Para las pruebas complejas y edge cases:
   python3 test_advanced.py 6667 mypassword
   ```

El script imprimirá por pantalla los comandos enviados (`>`) y las respuestas recibidas (`<`).

## Pruebas incluidas actualmente en `test_irc.py`:
- Autenticación (`PASS`, `NICK`, `USER`)
- Canales (`JOIN`, `PRIVMSG`)
- Modos de Operador (`MODE +t`, `TOPIC`)

Puedes ampliar el archivo `test_irc.py` para añadir el resto de los modos (`+i`, `+k`, `+l`, `+o`), así como los comandos `KICK` e `INVITE`.

