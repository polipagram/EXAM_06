#!/usr/bin/env python3

import socket
import subprocess
import time


HOST = "127.0.0.1"
PORT = 6667


def ok(name):
    print("[PASS] " + name)


def fail(name, received=None):
    print("[FAIL] " + name)
    if received is not None:
        print("       received:", repr(received))


def connect_client():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((HOST, PORT))
    s.settimeout(0.3)
    return s


def recv_all(s):
    data = b""

    while True:
        try:
            part = s.recv(4096)

            if not part:
                break

            data += part

            if len(part) < 4096:
                break

        except socket.timeout:
            break

    return data


def check(name, received, expected):
    if received == expected:
        ok(name)
        return True

    fail(name, received)
    print("       expected:", repr(expected))
    return False


def main():
    print("=== MINI_SERV EXAM TESTER ===")

    # ==================================================
    # 1. ARGUMENT VALIDATION
    # ==================================================

    print("\n1. ARGUMENT VALIDATION")

    p = subprocess.run(
        ["./mini_serv"],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE
    )

    if p.returncode == 1 and p.stderr == b"Wrong number of arguments\n":
        ok("wrong number of arguments")
    else:
        fail("wrong number of arguments")
        print("       stderr:", repr(p.stderr))

    # ==================================================
    # START SERVER
    # ==================================================

    print("\nStarting server...")

    server = subprocess.Popen(
        ["./mini_serv", str(PORT)],
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL
    )

    time.sleep(0.3)

    clients = []

    try:

        # ==================================================
        # 2. FIRST CLIENT
        # ==================================================

        print("\n2. FIRST CLIENT")

        c0 = connect_client()
        clients.append(c0)

        check(
            "client 0 receives no self-arrival",
            recv_all(c0),
            b""
        )

        # ==================================================
        # 3. ARRIVAL BROADCAST
        # ==================================================

        print("\n3. ARRIVAL BROADCAST")

        c1 = connect_client()
        clients.append(c1)

        time.sleep(0.1)

        check(
            "client 0 receives client 1 arrival",
            recv_all(c0),
            b"server: client 1 just arrived\n"
        )

        check(
            "client 1 receives no self-arrival",
            recv_all(c1),
            b""
        )

        # ==================================================
        # 4. CLIENT IDS
        # ==================================================

        print("\n4. CLIENT IDS")

        c2 = connect_client()
        clients.append(c2)

        time.sleep(0.1)

        expected = b"server: client 2 just arrived\n"

        check(
            "client 0 receives client 2 arrival",
            recv_all(c0),
            expected
        )

        check(
            "client 1 receives client 2 arrival",
            recv_all(c1),
            expected
        )

        check(
            "client 2 receives no self-arrival",
            recv_all(c2),
            b""
        )

        # ==================================================
        # 5. NORMAL MESSAGE
        # ==================================================

        print("\n5. NORMAL MESSAGE")

        c0.sendall(b"hello\n")
        time.sleep(0.1)

        expected = b"client 0: hello\n"

        check(
            "client 1 receives message",
            recv_all(c1),
            expected
        )

        check(
            "client 2 receives message",
            recv_all(c2),
            expected
        )

        check(
            "sender does not receive own message",
            recv_all(c0),
            b""
        )

        # ==================================================
        # 6. MULTIPLE LINES
        # ==================================================

        print("\n6. MULTIPLE LINES")

        c0.sendall(b"one\ntwo\nthree\n")
        time.sleep(0.1)

        expected = (
            b"client 0: one\n"
            b"client 0: two\n"
            b"client 0: three\n"
        )

        check(
            "multiple lines in one recv",
            recv_all(c1),
            expected
        )

        check(
            "multiple lines reach all clients",
            recv_all(c2),
            expected
        )

        # ==================================================
        # 7. PARTIAL MESSAGE
        # ==================================================

        print("\n7. PARTIAL MESSAGE")

        c0.sendall(b"hel")
        time.sleep(0.1)

        check(
            "partial message is buffered",
            recv_all(c1),
            b""
        )

        c0.sendall(b"lo\n")
        time.sleep(0.1)

        check(
            "partial message completed",
            recv_all(c1),
            b"client 0: hello\n"
        )

        # ==================================================
        # 8. PARTIAL + MULTIPLE LINES
        # ==================================================

        print("\n8. PARTIAL + MULTIPLE LINES")

        c0.sendall(b"first")
        time.sleep(0.05)

        check(
            "first partial part is buffered",
            recv_all(c1),
            b""
        )

        c0.sendall(b" message\nsecond\nthird\n")
        time.sleep(0.1)

        expected = (
            b"client 0: first message\n"
            b"client 0: second\n"
            b"client 0: third\n"
        )

        check(
            "partial followed by multiple lines",
            recv_all(c1),
            expected
        )

        # ==================================================
        # 9. EMPTY LINES
        # ==================================================

        print("\n9. EMPTY LINES")

        c0.sendall(b"\n")
        time.sleep(0.1)

        check(
            "single empty line",
            recv_all(c1),
            b"client 0: \n"
        )

        c0.sendall(b"\n\n\n")
        time.sleep(0.1)

        check(
            "multiple empty lines",
            recv_all(c1),
            b"client 0: \n"
            b"client 0: \n"
            b"client 0: \n"
        )

        # ==================================================
        # 10. BUFFERED NEWLINE
        # ==================================================

        print("\n10. BUFFERED NEWLINE")

        c0.sendall(b"abc")
        time.sleep(0.05)

        check(
            "text without newline is buffered",
            recv_all(c1),
            b""
        )

        c0.sendall(b"\n\n")
        time.sleep(0.1)

        check(
            "buffered text followed by empty line",
            recv_all(c1),
            b"client 0: abc\n"
            b"client 0: \n"
        )

        # ==================================================
        # 11. LARGE MESSAGE
        # ==================================================

        print("\n11. LARGE MESSAGE")

        large = b"A" * 2000 + b"\n"

        c0.sendall(large)
        time.sleep(0.2)

        expected = b"client 0: " + large

        check(
            "2000 byte message",
            recv_all(c1),
            expected
        )

        # ==================================================
        # 12. CLIENT-TO-CLIENT
        # ==================================================

        print("\n12. CLIENT-TO-CLIENT")

        # Drain anything left from previous tests.
        recv_all(c0)
        recv_all(c1)
        recv_all(c2)

        c1.sendall(b"from client 1\n")
        time.sleep(0.1)

        expected = b"client 1: from client 1\n"

        check(
            "client 0 receives client 1 message",
            recv_all(c0),
            expected
        )

        check(
            "client 2 receives client 1 message",
            recv_all(c2),
            expected
        )

        check(
            "client 1 does not receive own message",
            recv_all(c1),
            b""
        )

        # ==================================================
        # 13. DISCONNECT
        # ==================================================

        print("\n13. DISCONNECT")

        c0.close()
        clients.remove(c0)

        time.sleep(0.1)

        expected = b"server: client 0 just left\n"

        check(
            "client 1 receives client 0 departure",
            recv_all(c1),
            expected
        )

        check(
            "client 2 receives client 0 departure",
            recv_all(c2),
            expected
        )

        # ==================================================
        # 14. ID MUST NOT BE REUSED
        # ==================================================

        print("\n14. ID REUSE")

        c3 = connect_client()
        clients.append(c3)

        time.sleep(0.1)

        expected = b"server: client 3 just arrived\n"

        check(
            "new client gets next ID",
            recv_all(c1),
            expected
        )

        check(
            "new client receives no self-arrival",
            recv_all(c3),
            b""
        )

        # ==================================================
        # 15. MANY CLIENTS
        # ==================================================

        print("\n15. MANY CLIENTS")

        extra = []

        for i in range(4, 9):
            c = connect_client()
            extra.append(c)
            clients.append(c)

        time.sleep(0.2)

        # Drain arrival messages.
        for c in clients:
            recv_all(c)

        c1.sendall(b"broadcast\n")
        time.sleep(0.2)

        expected = b"client 1: broadcast\n"

        passed = 0

        for c in clients:
            if c is not c1:
                if recv_all(c) == expected:
                    passed += 1

        if passed == len(clients) - 1:
            ok("broadcast reaches every other client")
        else:
            fail("broadcast reaches every other client")
            print(
                "       received by:",
                passed,
                "/",
                len(clients) - 1
            )

        check(
            "sender excluded from broadcast",
            recv_all(c1),
            b""
        )

        # ==================================================
        # 16. LAZY CLIENT
        # ==================================================

        print("\n16. LAZY CLIENT")

        # c3 intentionally does not read.
        for i in range(100):
            c1.sendall(b"lazy test\n")

        time.sleep(0.2)

        if server.poll() is None:
            ok("server survives lazy client")
        else:
            fail("server died with lazy client")

        # ==================================================
        # 17. SERVER STILL ACCEPTS
        # ==================================================

        print("\n17. SERVER SURVIVAL")

        c9 = connect_client()
        clients.append(c9)

        time.sleep(0.1)

        if server.poll() is None:
            ok("server still accepts after stress")
        else:
            fail("server stopped after stress")

        # ==================================================
        # 18. REPEATED CONNECT / DISCONNECT
        # ==================================================

        print("\n18. REPEATED CONNECT / DISCONNECT")

        for i in range(5):
            temp = connect_client()
            time.sleep(0.02)
            temp.close()
            time.sleep(0.02)

        if server.poll() is None:
            ok("repeated connect/disconnect")
        else:
            fail("server died during repeated connect/disconnect")

        # ==================================================
        # 19. FINAL SERVER CHECK
        # ==================================================

        print("\n19. FINAL SERVER CHECK")

        if server.poll() is None:
            ok("server is still running")
        else:
            fail("server is no longer running")

    except Exception as e:
        print("[FAIL] tester exception:", e)

    finally:
        for c in clients:
            try:
                c.close()
            except:
                pass

        server.terminate()

        try:
            server.wait(timeout=2)
        except subprocess.TimeoutExpired:
            server.kill()
            server.wait()

    print("\n=== TEST FINISHED ===")


if __name__ == "__main__":
    main()