import os
import sys
import time
import struct
import socket

PORT_NO = 5000

def config_read (config):
    a = [];
    try:
        with open(config) as c:
            for line in c:
                line = line.strip()
                if line and not line.startswith("#"):
                    a.append([int (x) for x in line.split()])
    except ValueError:
        print("config file has a non number")
        return None
    if len(a) == 0:
        print("config file is empty")
        return None
    n = len(a[0]) + 1
    if len(a) != n - 1 or n > 26:
        print("config file has wrong number of rows")
        return None
    cost = [[-1] * n for i in range(n)]
    for i in range(n):
        cost[i][i] = 0
    for r, row in enumerate(a):
        if len(row) != n - 1 - r:
            print(f"row {r + 1} of config file should have {n - 1 - r} entries")
            return None
        for p, c in enumerate(row):
            if c == 0 or c < -1:
                print(f"bad cost {c} in config file")
                return None
            cost[r][r + p + 1] = cost[r + p + 1][r] = c
    return cost

def link_state(index, cost, addrs):
    tuples = [struct.pack("!c4sHI", chr(ord("A") + j).encode(),
                          socket.inet_aton(addrs[j][0]), addrs[j][1], c)
              for j, c in enumerate(cost[index]) if c >= 0]
    return struct.pack("!B", len(tuples)) + b"".join(tuples)

def send_all(conns, cost, addrs):
    for i, conn in enumerate(conns):
        try:
            conn.sendall(link_state(i, cost, addrs))
        except OSError:
            print(f"could not send to {chr(ord('A') + i)}")
    print("sent link states to all the VNs")

def my_ip():
    for target in ["8.8.8.8", "10.255.255.255", "172.31.255.255", "192.168.255.255"]:
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        try:
            s.connect((target, 80))
            ip = s.getsockname()[0]
            if not ip.startswith("127.") and ip != "0.0.0.0":
                return ip
        except OSError:
            pass
        finally:
            s.close()
    print("could not find my ip, give it as the second argument")
    sys.exit(1)

def main():
    if len(sys.argv) < 2:
        print("usage: python3 on.py <config_file> [my_ip]")
        return
    config = sys.argv[1]
    self_ip = sys.argv[2] if len(sys.argv) > 2 else my_ip()
    cost = config_read (config)
    if cost is None:
        return
    n = len(cost)

    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server.bind((self_ip, PORT_NO))
    server.listen(26)
    print(f"ON now listening, {self_ip}:{PORT_NO}, waiting for {n} VNs")

    conns, addrs = [], []
    for i in range(n):
        conn, _ = server.accept()
        ip, udp_port = struct.unpack("!4sH", conn.recv(6, socket.MSG_WAITALL))
        conns.append(conn)
        addrs.append((socket.inet_ntoa(ip), udp_port))
        print(f"{chr(ord('A') + i)} = {addrs[i][0]}:{udp_port}")

    send_all(conns, cost, addrs)

    islands = []
    last = os.path.getmtime(config)
    server.settimeout(1)
    while True:
        try:
            conn, _ = server.accept()
            msg = conn.recv(6, socket.MSG_WAITALL)
            letter = chr(ord('A') + n + len(islands))
            islands.append(conn)
            conn.sendall(struct.pack("!Bc", 1, letter.encode()) + msg + struct.pack("!I", 0))
            print(f"{letter} = {socket.inet_ntoa(msg[:4])}:{struct.unpack('!H', msg[4:])[0]} (island)")
        except socket.timeout:
            pass

        if os.path.getmtime(config) != last:
            last = os.path.getmtime(config)
            new_cost = config_read (config)
            if new_cost is None or len(new_cost) != n:
                print("ignoring bad config change")
                continue
            cost = new_cost
            print("config file changed")
            send_all(conns, cost, addrs)

main()
