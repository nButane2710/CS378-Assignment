#!/bin/bash
# usage: bash tests/run_vns.sh <on_ip> <first_port> <how_many> [my_ip]
cd "$(dirname "$0")/.."
ON_IP=$1
MY_IP=${4:-$(ipconfig getifaddr en0 2>/dev/null || hostname -I | awk '{print $1}')}
make -s || exit 1
mkdir -p logs
for i in $(seq 0 $(($3 - 1))); do
    ./vn/vn $MY_IP $(($2 + i)) $ON_IP > logs/vn_${MY_IP}_$(($2 + i)).log 2>&1 &
    echo "started vn $MY_IP:$(($2 + i))"
    sleep 1
done
echo "logs are in logs/, ctrl-c to stop"
wait
