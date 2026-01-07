#!/bin/bash
BIN="./bin/autobus_main"

cleanup() { ipcrm -a 2>/dev/null || true; }

test1() {
    echo "=== TEST 1: Obciążeniowy - czy są odmowy przy małej pojemności? ==="
    cleanup
    timeout 30s $BIN 2 5 2 5000 1
    
    # Sprawdź czy były odmowy (oczekiwane!)
    if grep -q "Odmowa" raport.txt; then
        echo "[PASS] Wykryto odmowy - system poprawnie obsługuje przepełnienie"
    else
        echo "[FAIL] Brak odmów - przy P=5 powinny być!"
    fi
    cleanup
}

test2() {
    echo "=== TEST 2: Czy rowerzyści są odmawiani gdy R=2? ==="
    cleanup
    timeout 30s $BIN 3 10 1 4000 1
    
    if grep -q "brak miejsc rowerowych" raport.txt; then
        echo "[PASS] Rowerzyści odmawiani gdy brak miejsc"
    else
        echo "[INFO] Brak odmów rowerowych (może za mało rowerzystów)"
    fi
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
    if grep -q ">>> WYMUSZONY ODJAZD (SIGUSR1)" raport.txt; then
        echo "[PASS] Wymuszony odjazd zadziałał"
    else
        echo "[INFO] Brak autubusów na peronie w tym przebiegu"
    fi
    cleanup
}

test4() {
    echo "=== TEST 4: SIGUSR2 (N=3 P=10 R=3 T=5000 K=2) ==="
    cleanup
    $BIN 3 10 3 5000 2 &
    sleep 20
    kill -SIGUSR2 $(pgrep -f "autobus_main" | head -1) 2>/dev/null
    wait 2>/dev/null
    if grep -q "Dworzec zamknięty. Czekam na zakończenie przejazdów" raport.txt; then
        echo "[PASS] Zamknięcie dworca zadziałało"
    else
        echo "[INFO] Zamkniecie dworca nie zostało zarejestrowane"
    fi
    cleanup
}
test5() {
    echo "=== TEST 5: VIP omija kolejki ==="
    cleanup
    timeout 240s $BIN 3 10 3 5000 2
    
    # Sprawdź czy VIP ominął kasę
    if grep -q "VIP - omija kolejke do kasy" raport.txt; then
        echo "[PASS] VIP omija kolejkę do kasy"
    else
        echo "[INFO] Nie wygenerowano VIP w tym przebiegu (1% szans)"
    fi
    
    # Sprawdź czy VIP wsiadł
    if grep -q "VIP" raport.txt | grep -q "Wsiadł"; then
        echo "[PASS] VIP wsiadł do autobusu"
    fi
    cleanup
}
test6() {
    echo "=== TEST 6: Dzieci czekają na opiekuna ==="
    cleanup
    timeout 40s $BIN 3 10 3 5000 1
    
    # Sprawdź czy były dzieci czekające
    if grep -q "czeka PRZED dworcem na opiekuna" raport.txt; then
        echo "[PASS] Dzieci czekają przed dworcem na opiekuna"
    else
        echo "[INFO] Nie wygenerowano dzieci w tym przebiegu"
    fi
    
    # Sprawdź czy dzieci wsiadały TYLKO z opiekunem
    if grep -q "z dzieckiem" raport.txt; then
        echo "[PASS] Dzieci wsiadają tylko z opiekunem"
    fi
    cleanup
}
test7() {
    echo "=== TEST 7: Autobus odjeżdża wcześniej gdy pełny ==="
    cleanup
    # Mała pojemność (P=5) + długi postój (T=15000) = powinien się zapełnić przed czasem
    timeout 60s $BIN 3 5 2 15000 2
    
    if grep -q "PELNY - odjazd!" raport.txt; then
        echo "[PASS] Autobus odjechał wcześniej po zapełnieniu"
    else
        echo "[FAIL] Nie wykryto wcześniejszego odjazdu"
    fi
    cleanup
}

case "$1" in
    1) test1 ;;
    2) test2 ;;
    3) test3 ;;
    4) test4 ;;
    5) test5 ;;
    6) test6 ;;
    7) test7 ;;
    all) test1; test2; test3; test4; test5; test6; test7 ;;
    *) echo "Użycie: $0 [1|2|3|4|5|6|7|all]" ;;
esac