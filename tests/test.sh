#!/bin/bash
BIN="./bin/autobus_main"

cleanup() { ipcrm -a 2>/dev/null || true; }

test1() {
    echo "=== TEST 1: Obciążeniowy (N=2 P=5 R=2 T=3000 K=1) ==="
    cleanup
    timeout 60s $BIN 2 5 2 3000 1 || true
    cleanup
}

test2() {
    echo "=== TEST 2: Rowerzyści (N=3 P=10 R=2 T=5000 K=2) ==="
    cleanup
    timeout 45s $BIN 3 10 2 5000 2 || true
    cleanup
}

test3() {
    echo "=== TEST 3: SIGUSR1 (N=2 P=10 R=3 T=15000 K=1) ==="
    cleanup
    $BIN 2 10 3 15000 1 &
    sleep 12
    kill -SIGUSR1 $(pgrep -f "autobus_main" | head -1) 2>/dev/null
    sleep 5
    kill -SIGUSR2 $(pgrep -f "autobus_main" | head -1) 2>/dev/null
    wait 2>/dev/null
    cleanup
}

test4() {
    echo "=== TEST 4: SIGUSR2 (N=3 P=10 R=3 T=5000 K=2) ==="
    cleanup
    $BIN 3 10 3 5000 2 &
    sleep 20
    kill -SIGUSR2 $(pgrep -f "autobus_main" | head -1) 2>/dev/null
    wait 2>/dev/null
    cleanup
}

case "$1" in
    1) test1 ;;
    2) test2 ;;
    3) test3 ;;
    4) test4 ;;
    all) test1; test2; test3; test4 ;;
    *) echo "Użycie: $0 [1|2|3|4|all]" ;;
esac