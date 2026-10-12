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

    socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    





         
           




