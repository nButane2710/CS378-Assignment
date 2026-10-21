#!/bin/bash
source "$(dirname "$0")/common.sh"

echo "=== more VNs than the config has (islands) ==="
start_on tests/configs/three.txt
start_vns 5
sleep 12
check "4th VN is an island" "$(head -1 logs/vn3.log)" "D=0"
check "5th VN is an island" "$(head -1 logs/vn4.log)" "E=0"
check "on log names islands" "$(grep -c island logs/on.log)" "2"
check_all_graphs tests/configs/three.txt 3 "with islands"
stop_all

echo "=== bad config file at start ==="
python3 on/on.py tests/configs/bad_rows.txt $IP > logs/on.log 2>&1
check "wrong row length" "$(cat logs/on.log)" "row 2 of config file should have 3 entries"
python3 on/on.py tests/configs/bad_value.txt $IP > logs/on.log 2>&1
check "non number" "$(cat logs/on.log)" "config file has a non number"

echo "=== bad config change while running ==="
start_on tests/configs/example.txt
start_vns 5
sleep 2
change_config tests/configs/bad_value.txt
sleep 12
check "on ignores bad change" "$(grep -c 'ignoring bad config change' logs/on.log)" "1"
check_all_graphs tests/configs/example.txt 5 "after bad change"

echo "=== VN killed, then config changes ==="
pkill -f "vn/vn $IP 6004"
sleep 1
change_config tests/configs/example_cost_change.txt
sleep 3
check "on still running" "$(kill -0 $ON_PID 2>/dev/null && echo yes)" "yes"
check "D still gets updates" "$(grep '^D=' logs/vn3.log | tail -1)" "D=0,B=4,E=2"

echo "=== ON stops, VNs exit ==="
kill $ON_PID
sleep 1
check "VN prints ON closed" "$(tail -1 logs/vn0.log)" "ON closed the connection"
check "no VNs left" "$(pgrep -f "vn/vn $IP" | wc -l | tr -d ' ')" "0"

echo "=== VN errors ==="
check "wrong args" "$(./vn/vn $IP)" "usage: ./vn <my_ip> <my_udp_port> <on_ip>"
if lsof -nP -iTCP:5000 -sTCP:LISTEN > /dev/null 2>&1; then
    echo "SKIP: ON not running (something else, e.g. AirPlay on mac, listens on port 5000)"
else
    check "ON not running" "$(./vn/vn $IP 6100 $IP 2>&1)" "connect: Connection refused"
fi
python3 -c "import socket,time; s=socket.socket(socket.AF_INET,socket.SOCK_DGRAM); s.bind(('$IP',6101)); time.sleep(3)" &
sleep 1
check "UDP port in use" "$(./vn/vn $IP 6101 $IP 2>&1)" "bind: Address already in use"
wait

finish
