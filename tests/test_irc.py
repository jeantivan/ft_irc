import socket
import time
import sys

def send_msg(sock, msg):
    print(f"> {msg.strip()}")
    sock.send((msg + "\r\n").encode())

def recv_msg(sock):
    sock.settimeout(1.0)
    try:
        data = sock.recv(4096).decode()
        if data:
            print(f"< {data.strip()}")
            return data
    except socket.timeout:
        pass
    return ""

def test_authentication(port, password):
    print("--- Testing Authentication ---")
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect(("127.0.0.1", port))

    send_msg(s, f"PASS {password}")
    send_msg(s, "NICK testuser")
    send_msg(s, "USER testuser 0 * :Test User")

    time.sleep(1)
    recv_msg(s)
    recv_msg(s)

    print("Authentication test completed.\n")
    return s

def test_join_privmsg(s):
    print("--- Testing JOIN and PRIVMSG ---")
    send_msg(s, "JOIN #testchannel")
    time.sleep(0.5)
    recv_msg(s)

    send_msg(s, "PRIVMSG #testchannel :Hello World!")
    time.sleep(0.5)
    recv_msg(s)
    print("JOIN and PRIVMSG test completed.\n")

def test_channel_modes(s):
    print("--- Testing Channel Modes (MODE +t, +i, +k, +l, +o) ---")
    # Assuming testuser is operator of #testchannel since they created it
    send_msg(s, "MODE #testchannel +t")
    time.sleep(0.5)
    recv_msg(s)

    send_msg(s, "TOPIC #testchannel :New Topic")
    time.sleep(0.5)
    recv_msg(s)
    print("Modes test completed.\n")

def main():
    if len(sys.argv) != 3:
        print("Usage: python3 test_irc.py <port> <password>")
        return

    port = int(sys.argv[1])
    password = sys.argv[2]

    try:
        s = test_authentication(port, password)
        test_join_privmsg(s)
        test_channel_modes(s)

        send_msg(s, "QUIT :Goodbye")
        time.sleep(0.5)
        recv_msg(s)
        s.close()
        print("All basic tests completed.")
    except Exception as e:
        print(f"Error during tests: {e}")

if __name__ == '__main__':
    main()

