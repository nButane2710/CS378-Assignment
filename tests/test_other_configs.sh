#!/bin/bash
source "$(dirname "$0")/common.sh"

echo "=== 3 nodes ==="
start_on tests/configs/three.txt
start_vns 3
sleep 12
check "A link state" "$(head -1 logs/vn0.log)" "A=0,B=4,C=9"
check_all_graphs tests/configs/three.txt 3 "three"
stop_all

echo "=== spaces instead of tabs ==="
start_on tests/configs/spaces.txt
start_vns 4
sleep 12
check "B link state" "$(head -1 logs/vn1.log)" "B=0,A=1,C=2"
check_all_graphs tests/configs/spaces.txt 4 "spaces"
stop_all

echo "=== node with no links ==="
start_on tests/configs/isolated.txt
start_vns 4
sleep 12
check "C has no neighbours" "$(head -1 logs/vn2.log)" "C=0"
want=$(matrix tests/configs/isolated.txt)
for i in 0 1 3; do
    check "isolated: graph at vn$i matches config" "$(last_graph $i)" "$want"
done
stop_all

echo "=== 26 nodes in a ring ==="
start_on tests/configs/ring26.txt
start_vns 26
sleep 22
check "A link state" "$(head -1 logs/vn0.log)" "A=0,B=1,Z=1"
check "Z link state" "$(head -1 logs/vn25.log)" "Z=0,A=1,Y=5"
check_all_graphs tests/configs/ring26.txt 26 "ring26"

finish
