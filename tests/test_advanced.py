import socket
import time
import sys

class IRCClient:
    def __init__(self, host, port):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.settimeout(2.0)
        self.sock.connect((host, port))

    def send(self, msg):
        print(f"[CLIENT] > {msg}")
        self.sock.send((msg + "\r\n").encode())

    def send_raw(self, data):
        print(f"[CLIENT RAW] > {data}")
        self.sock.send(data)

    def recv_all(self):
        try:
            data = self.sock.recv(4096).decode()
            if data:
                for line in data.split("\r\n"):
                    if line:
                        print(f"[SERVER] < {line}")
            return data
        except socket.timeout:
            return ""

    def close(self):
        self.sock.close()

def setup_client(port, password, nick, user):
    client = IRCClient("127.0.0.1", port)
    client.send(f"PASS {password}")
    client.send(f"NICK {nick}")
    client.send(f"USER {user} 0 * :Test User {nick}")
    time.sleep(0.5)
    client.recv_all()
    return client

def test_wrong_password(port, password):
    print("\n--- Test: Contraseña Incorrecta ---")
    client = IRCClient("127.0.0.1", port)
    client.send("PASS wrongpass")
    client.send("NICK wrong")
    client.send("USER wrong 0 * :Wrong")
    time.sleep(0.5)
    client.recv_all()
    client.close()

def test_missing_params(port, password):
    print("\n--- Test: Faltan Parámetros ---")
    client = setup_client(port, password, "noparam", "noparam")
    client.send("JOIN")
    client.send("PRIVMSG")
    client.send("MODE")
    time.sleep(0.5)
    client.recv_all()
    client.close()

def test_nick_collision(port, password):
    print("\n--- Test: Colisión de Nicknames ---")
    c1 = setup_client(port, password, "same_nick", "user1")
    c2 = IRCClient("127.0.0.1", port)
    c2.send(f"PASS {password}")
    c2.send("NICK same_nick")
    c2.send("USER user2 0 * :User 2")
    time.sleep(0.5)
    c2.recv_all()
    c1.close()
    c2.close()

def test_channel_limits_and_keys(port, password):
    print("\n--- Test: Límites (+l) y Contraseñas de Canal (+k) ---")
    c1 = setup_client(port, password, "op_lim", "user1")
    c2 = setup_client(port, password, "norm_lim", "user2")
    c3 = setup_client(port, password, "ext_usr", "user3")

    c1.send("JOIN #limitchan")
    time.sleep(0.2)
    c1.recv_all()

    # c1 asigna una contraseña al canal
    c1.send("MODE #limitchan +k mysecret")
    time.sleep(0.2)

    # c2 intenta entrar sin la contraseña (debe fallar)
    c2.send("JOIN #limitchan")
    time.sleep(0.2)
    c2.recv_all()

    # c2 intenta entrar con la contraseña (debe funcionar)
    c2.send("JOIN #limitchan mysecret")
    time.sleep(0.2)
    c2.recv_all()

    # c1 establece un límite de 2 usuarios
    c1.send("MODE #limitchan +l 2")
    time.sleep(0.2)

    # c3 intenta entrar cuando el canal ya está lleno (debe fallar)
    c3.send("JOIN #limitchan mysecret")
    time.sleep(0.2)
    c3.recv_all()

    c1.close()
    c2.close()
    c3.close()

def test_invite_only(port, password):
    print("\n--- Test: Canal de Solo Invitación (+i) e INVITE ---")
    c1 = setup_client(port, password, "op_inv", "user1")
    c2 = setup_client(port, password, "tgt_inv", "user2")

    c1.send("JOIN #invchan")
    time.sleep(0.2)
    # c1 hace el canal invite-only
    c1.send("MODE #invchan +i")
    time.sleep(0.2)
    c1.recv_all()

    # c2 intenta entrar sin ser invitado (debe fallar)
    c2.send("JOIN #invchan")
    time.sleep(0.2)
    c2.recv_all()

    # c1 invita a c2
    c1.send("INVITE tgt_inv #invchan")
    time.sleep(0.2)
    c1.recv_all()
    c2.recv_all()

    # c2 intenta entrar de nuevo (ahora debe funcionar)
    c2.send("JOIN #invchan")
    time.sleep(0.2)
    c2.recv_all()

    c1.close()
    c2.close()

def test_kick_and_topic(port, password):
    print("\n--- Test: KICK y Restricción de Topic (+t) ---")
    c1 = setup_client(port, password, "op_kick", "user1")
    c2 = setup_client(port, password, "tgt_kick", "user2")

    c1.send("JOIN #kickchan")
    c2.send("JOIN #kickchan")
    time.sleep(0.2)
    c1.recv_all()
    c2.recv_all()

    # c1 restringe quién puede cambiar el topic
    c1.send("MODE #kickchan +t")
    time.sleep(0.2)

    # c2 intenta cambiar el topic (debe fallar porque no es op)
    c2.send("TOPIC #kickchan :New topic by normal user")
    time.sleep(0.2)
    c2.recv_all()

    # c1 cambia el topic
    c1.send("TOPIC #kickchan :Op topic")
    time.sleep(0.2)
    c1.recv_all()

    # c2 intenta echar (KICK) a c1 (debe fallar)
    c2.send("KICK #kickchan op_kick :Get out")
    time.sleep(0.2)
    c2.recv_all()

    # c1 echa a c2
    c1.send("KICK #kickchan tgt_kick :Bye")
    time.sleep(0.2)
    c1.recv_all()
    c2.recv_all()

    c1.close()
    c2.close()

def test_operator_privileges(port, password):
    print("\n--- Test: Dar/Quitar Privilegios de Operador (+o) ---")
    c1 = setup_client(port, password, "admin_usr", "user1")
    c2 = setup_client(port, password, "pleb_usr", "user2")

    c1.send("JOIN #opchan")
    c2.send("JOIN #opchan")
    time.sleep(0.5)

    # c2 intenta echar a c1 (debe fallar)
    c2.send("KICK #opchan admin_usr :Bye")
    time.sleep(0.2)
    c2.recv_all()

    # c1 le da op a c2
    c1.send("MODE #opchan +o pleb_usr")
    time.sleep(0.2)
    c1.recv_all()

    # c2 ahora es op y echa a c1 (debe funcionar)
    c2.send("KICK #opchan admin_usr :Now I am the captain")
    time.sleep(0.2)
    c2.recv_all()

    c1.close()
    c2.close()

def test_partial_data(port, password):
    print("\n--- Test: Datos Parciales (Agregación de Paquetes TCP) ---")
    c1 = IRCClient("127.0.0.1", port)

    # Enviar password fragmentado
    c1.send_raw(b"PASS " + password.encode() + b"\r\n")

    # Enviar NICK en 3 partes
    c1.send_raw(b"NICK part")
    time.sleep(0.5)
    c1.send_raw(b"i")
    time.sleep(0.5)
    c1.send_raw(b"al\r\n")

    c1.send_raw(b"USER pt pt * :pt\r\n")
    time.sleep(0.5)
    c1.recv_all()

    # Enviar JOIN en 3 partes
    c1.send_raw(b"JOIN ")
    time.sleep(0.5)
    c1.send_raw(b"#")
    time.sleep(0.5)
    c1.send_raw(b"partial\r\n")
    time.sleep(0.5)
    c1.recv_all()
    c1.close()

def main():
    if len(sys.argv) != 3:
        print("Usage: python3 test_advanced.py <port> <password>")
        return

    port = int(sys.argv[1])
    password = sys.argv[2]

    try:
        test_wrong_password(port, password)
        test_missing_params(port, password)
        test_nick_collision(port, password)
        test_channel_limits_and_keys(port, password)
        test_invite_only(port, password)
        test_kick_and_topic(port, password)
        test_operator_privileges(port, password)
        test_partial_data(port, password)

        print("\n=== Todos los tests avanzados finalizados ===")
    except Exception as e:
        print(f"Error during tests: {e}")

if __name__ == '__main__':
    main()

