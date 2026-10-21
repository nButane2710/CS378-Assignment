#!/bin/bash
source "$(dirname "$0")/common.sh"

IP2=$( (ifconfig 2>/dev/null | grep 'inet ' | awk '{print $2}'; hostname -I 2>/dev/null | tr ' ' '\n') | grep -v '^127\.' | grep -v "^$IP$" | grep . | head -1)
if [ -z "$IP2" ]; then
    echo "SKIP: this machine has only one IP"
    exit 0
fi

works=$(python3 -c "
import socket
r = socket.socket(socket.AF_INET, socket.SOCK_DGRAM); r.bind(('$IP2', 0)); r.settimeout(1)
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM); s.bind(('$IP', 0)); s.sendto(b'x', r.getsockname())
try: r.recv(10); print('yes')
except OSError: print('no')")
if [ "$works" != "yes" ]; then
    echo "SKIP: second ip $IP2 can't carry UDP (probably a VPN), use tests/run_vns.sh on two machines instead"
    exit 0
fi

echo "=== A,B on $IP and C,D,E on $IP2 (like the pdf figure) ==="
start_on tests/configs/example.txt
ips=($IP $IP $IP2 $IP2 $IP2)
for i in 0 1 2 3 4; do
    ./vn/vn ${ips[$i]} $((6000 + i)) $IP > logs/vn$i.log 2>&1 &
    while [ $(grep -c " = " logs/on.log) -le $i ]; do sleep 0.1; done
done
sleep 12
check "C registered with second ip" "$(grep '^C = ' logs/on.log)" "C = $IP2:6002"
check "E link state" "$(head -1 logs/vn4.log)" "E=0,C=1,D=7"
check_all_graphs tests/configs/example.txt 5 "two ips"
stop_all
pkill -f "vn/vn $IP2" 2>/dev/null

finish
