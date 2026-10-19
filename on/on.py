import os
import sys
import socket

PORT_NO = 5000

def config_read (config):
    a = [];
    with open(config) as c:
        for line in c:
            line = line.strip()
            if line and not line.startswith("#"):
                rows.append([int (x) for x in line.split()])
    n = len(a[0]) + 1
    cost = [[-1] * n for i in range(n)]
    for i in range(n):
        cost[i][i] = 0
    for r, row in enumerate(a):
        for p, c in enumerate(row):
            cost[r][r + p + 1] = cost[r + p + 1][r] = c
    return cost

def link_state(index, cost, addrs):
    tuples = [struct.pack("!c4sHI", chr(ord("A") + j).encode(),
                          socket.inet_aton(addrs[j][0]), addrs[j][1], c)
              for j, c in enumerate(cost[index]) if c >= 0]
    return struct.pack("!B", len(tuples)) + b"".join(tuples)

def main():
    config, self_ip = sys.argv[1], sys.argv[2]
    cost = config_read (config)
    n = len(cost)

    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
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

    
    for i, conn in enumerate(conns):
        conn.sendall(link_state(i, cost, addrs))
    print("sent link states to all the VNs")







         
           




