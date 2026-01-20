//Mateusz Janecki 155182 - projekt SO 2025/2026 Autobus Podmiejski
//Parsuje argumenty i uruchamia dyspozytora
//Użycie: ./autobus_main [N] [P] [R] [T] [K]
#include "dyspozytor.h"
#include "common.h"

int main(int argc, char *argv[]) {
    //Domyslne parametry symulacji
    int N = 5;//Liczba autobusow
    int P = 10;//Pojemnosc
    int R = 3;//Rowery
    int T = 5000;//Postoj w milisekundach
    int K = 1;//Liczba kas

    //parsowanie argumentow
    if (argc > 1) N = atoi(argv[1]);
    if (argc > 2) P = atoi(argv[2]);
    if (argc > 3) R = atoi(argv[3]);
    if (argc > 4) T = atoi(argv[4]);
    if (argc > 5) K = atoi(argv[5]);

    //walidacja
    if (N <= 0 || N > MAX_BUSES) {
        fprintf(stderr, "Blad: N musi byc 1-%d\n", MAX_BUSES);
        return 1;
    }
    if (P <= 0 || P > MAX_CAPACITY) {
        fprintf(stderr, "Blad: P musi byc 1-%d\n", MAX_CAPACITY);
        return 1;
    }
    if (R < 0 || R > P) {
        fprintf(stderr, "Blad: R musi byc 0-%d\n", P);
        return 1;
    }
    if (T <= 0) {
        fprintf(stderr, "Blad: T musi byc > 0\n");
        return 1;
    }
    if (K <= 0 || K > MAX_KASY) {
        fprintf(stderr, "Blad: K musi byc 1-%d\n", MAX_KASY);
        return 1;
    }
    //start symulacji głowna petla symulacji
    proces_dyspozytor(N, P, R, T, K);
    return 0;
}
