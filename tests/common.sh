#!/bin/bash

cd "$(dirname "$0")/.."
IP=$(ipconfig getifaddr en0 2>/dev/null || hostname -I | awk '{print $1}')
gcc vn/vn.c -o vn/vn || exit 1
mkdir -p logs
PASS=0
FAIL=0

check() {
    if [ "$2" == "$3" ]; then
        echo "PASS: $1"
        PASS=$((PASS + 1))
    else
        echo "FAIL: $1"
        echo "  expected: $3"
        echo "  got:      $2"
        FAIL=$((FAIL + 1))
    fi
}

start_on() {
    cp $1 logs/config.txt
    python3 -u on/on.py logs/config.txt $IP > logs/on.log 2>&1 &
    ON_PID=$!
    sleep 1
}

start_vns() {
    for i in $(seq 0 $(($1 - 1))); do
        ./vn/vn $IP $((6000 + i)) $IP > logs/vn$i.log 2>&1 &
        while [ $(grep -c " = " logs/on.log) -le $i ]; do sleep 0.1; done
    done
}

change_config() {
    cp $1 logs/config.txt
}

matrix() {
    grep -v '^#' $1 | awk 'NF {$1=$1; print}'
}

last_graph() {
    awk '/^graph at/ {g=""; next} /^end/ {last=g; next} {g=g $0 "\n"} END {printf "%s", last}' logs/vn$1.log | awk '{$1=$1; print}'
}

check_all_graphs() {
    want=$(matrix $1)
    for i in $(seq 0 $(($2 - 1))); do
        check "$3: graph at vn$i matches config" "$(last_graph $i)" "$want"
    done
}

stop_all() {
    kill $ON_PID 2>/dev/null
    sleep 1
    pkill -f "vn/vn $IP" 2>/dev/null
}

finish() {
    stop_all
    echo "$PASS passed, $FAIL failed"
    [ $FAIL -eq 0 ]
}
