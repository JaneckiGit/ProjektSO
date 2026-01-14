#!/bin/bash
BIN="./bin/autobus_main"

cleanup() { ipcrm -a 2>/dev/null || true; }

test1() {
    echo "=== TEST 1: Obciążeniowy - czy są odmowy przy małej pojemności? ==="
    cleanup
    timeout 30s $BIN 2 5 2 5000 1
    if grep -q "Odmowa" raport.txt; then
        echo "[PASS] Wykryto odmowy - system poprawnie obsługuje przepełnienie"
    else
        echo "[FAIL] Brak odmów - przy P=5 powinny być!"
    fi
    cleanup
}

test2() {
    echo "=== TEST 2: Czy są odmowy przy rowerzystach R=1? ==="
    cleanup
    timeout 30s $BIN 3 10 1 5000 1
    if grep -q "brak miejsc rowerowych" raport.txt; then
        echo "[PASS] odmowy dla rowerzystów - brak miejsc"
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
    echo "=== TEST 5: Autobus odjeżdża wcześniej gdy pełny ==="
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

test6() {
    echo "=== TEST 6: Zamknięcie drzwi przed odjazdem ==="
    cleanup
    timeout 40s $BIN 2 8 3 6000 1
        zamkniecia=$(grep -c "ZAMYKAM DRZWI PRZED ODJAZDEM" raport.txt || echo 0)
    odjazdy=$(grep -c "ODJAZD! Zabral" raport.txt || echo 0)
    
    if [ "$zamkniecia" -gt 0 ]; then
        echo "[PASS] Znaleziono $zamkniecia zamknięć drzwi (odjazdy: $odjazdy)"

    if grep -A1 "ZAMYKAM DRZWI PRZED ODJAZDEM" raport.txt | grep -q "ODJAZD!"; then
            echo "[PASS] Poprawna kolejność: zamknięcie drzwi -> odjazd"
        else
            echo "[INFO] Nie można zweryfikować kolejności"
        fi
    else
        echo "[FAIL] Brak komunikatu zamknięcia drzwi"
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
    all) test1; echo ""; test2; echo ""; test3; echo ""; test4; echo ""; test5; echo ""; test6 ;;
    *) echo "Użycie: $0 [1-6|all]" ;;
esac