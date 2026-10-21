#!/bin/bash
cd "$(dirname "$0")"
ok=0
for t in test_example.sh test_other_configs.sh test_errors.sh test_partition.sh test_two_ips.sh; do
    bash $t || ok=1
    echo
done
if [ $ok -eq 0 ]; then echo "ALL TESTS PASSED"; else echo "SOME TESTS FAILED"; fi
exit $ok
