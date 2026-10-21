#!/bin/bash
source "$(dirname "$0")/common.sh"

echo "=== example network from the pdf, then config changes ==="
start_on tests/configs/example.txt
start_vns 5
sleep 12

check "A link state" "$(head -1 logs/vn0.log)" "A=0,B=3,C=1"
check "B link state" "$(head -1 logs/vn1.log)" "B=0,A=3,C=2,D=4"
check "C link state" "$(head -1 logs/vn2.log)" "C=0,A=1,B=2,E=1"
check "D link state" "$(head -1 logs/vn3.log)" "D=0,B=4,E=7"
check "E link state" "$(head -1 logs/vn4.log)" "E=0,C=1,D=7"
check_all_graphs tests/configs/example.txt 5 "start"

change_config tests/configs/example_cost_change.txt
sleep 12
check "E after cost change" "$(grep '^E=' logs/vn4.log | tail -1)" "E=0,C=1,D=2"
check "D after cost change" "$(grep '^D=' logs/vn3.log | tail -1)" "D=0,B=4,E=2"
check_all_graphs tests/configs/example_cost_change.txt 5 "cost change"

change_config tests/configs/example_link_removed.txt
sleep 12
check "B after link removed" "$(grep '^B=' logs/vn1.log | tail -1)" "B=0,A=3,C=2"
check "D after link removed" "$(grep '^D=' logs/vn3.log | tail -1)" "D=0,E=2"
check_all_graphs tests/configs/example_link_removed.txt 5 "link removed"

change_config tests/configs/example_link_added.txt
sleep 12
check "A after link added" "$(grep '^A=' logs/vn0.log | tail -1)" "A=0,B=3,C=1,D=5"
check "D after link added" "$(grep '^D=' logs/vn3.log | tail -1)" "D=0,A=5,E=2"
check_all_graphs tests/configs/example_link_added.txt 5 "link added"

finish
