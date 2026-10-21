#!/bin/bash
source "$(dirname "$0")/common.sh"

echo "=== network splits, old entries must be forgotten ==="
start_on tests/configs/example.txt
start_vns 5
sleep 12
check_all_graphs tests/configs/example.txt 5 "before split"

change_config tests/configs/example_partition.txt
sleep 45
check "E has no neighbours" "$(grep '^E=' logs/vn4.log | tail -1)" "E=0"
want=$(printf '3 1 -1\n2 4\n-1')
for i in 0 1 2 3; do
    check "graph at vn$i forgot E" "$(last_graph $i)" "$want"
done
check "graph at vn4 forgot everyone" "$(last_graph 4)" "$(printf -- '-1 -1 -1 -1\n-1 -1 -1\n-1 -1\n-1')"

change_config tests/configs/example.txt
sleep 12
check_all_graphs tests/configs/example.txt 5 "after rejoin"

finish
