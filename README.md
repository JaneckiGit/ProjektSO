## Mateusz Janecki
Nr. Albumu: 155182
Kierunek: Informatyka
Rok: 2
Semestr: 3
Grupa Projektowa: 1
Przedmiot: Systemy Operacyjne (Projekt)

## Link do GitHub:
https://github.com/JaneckiGit/

## LInk do REPOZYTORIUM PROJEKTU
https://github.com/JaneckiGit/ProjektSO/

## Wybór Projektu: 
Temat 12 
Autobus podmiejski. 
Na dworcu stoi autobus o pojemności P pasażerów, w którym jednocześnie można przewieźć R rowerów. Do autobusu są dwa wejścia (każde może pomieścić jedną osobę): jedno dla pasażerów z bagażem podręcznym i drugie dla pasażerów z rowerami. Autobus odjeżdża co określoną ilość czasu T (np.: co 10 minut). W momencie odjazdu kierowca musi dopilnować aby na wejściach nie było żadnego wchodzącego pasażera. Jednocześnie musi dopilnować by liczba wszystkich pasażerów autobusu nie przekroczyła P i liczba pasażerów z rowerami nie była większa niż R. Po odjeździe autobusu na jego miejsce pojawia się natychmiast (jeżeli jest dostępny) nowy pojazd o takiej samej pojemności jak poprzedni. Łączna liczba autobusów wynosi N, każdy o pojemności P z R miejscami na rowery. Pasażerowie w różnym wieku przychodzą na dworzec w losowych momentach czasu. Przed wejściem na przystanek kupują bilet w kasie (K). Istnieje pewna liczba osób VIP (ok. 1% ) – posiadająca wcześniej wykupiony bilet, które nie płacą za bilet i mogą wejść na przystanek i do autobusu z pominięciem kolejki oczekujących. Dzieci w wieku poniżej 8 roku życia mogą wejść do autobusu tylko pod opieką osoby dorosłej (dziecko zajmuje osobne miejsce w autobusie). Kasa rejestruje wszystkie wchodzące osoby (ID procesu/wątku). Każdy pasażer przy wejściu do autobusu okazuje kierowcy ważny bilet – do autobusu nie może wejść osoba bez wykupionego wcześniej biletu. Autobusy przewożą pasażerów do miejsca docelowego i po czasie Ti (wartość losowa, każdy autobus ma inny czas) wracają na dworzec. Na polecenie dyspozytora (sygnał 1) autobus, który w danym momencie stoi na dworcu może odjechać z niepełną liczbą pasażerów. Po otrzymaniu od dyspozytora polecenia (sygnał 2) pasażerowie nie mogą wsiąść do żadnego autobusu - nie mogą wejść na dworzec. Autobusy kończą pracę po rozwiezieniu wszystkich pasażerów. Napisz programy symulujące działanie dyspozytora, kasy, kierowcy i pasażerów. Raport z przebiegu symulacji zapisać w pliku (plikach) tekstowym.

## Struktura projektu
```
ProjektSO/
├── src/              # Pliki źródłowe
│   ├── main.c        # Proces główny (Director)
│   ├── bus.c         # Proces autobusu/kierowcy
│   ├── dyspozytor.c  # Proces dyspozytora
│   ├── kasa.c        # Proces kasy biletowej
│   ├── utils.c       # 
│   └── pasazer.c     # Proces pasażera
├── include/          # Pliki nagłówkowe
│   ├── common.h      # Definicje wspólne(definicje etc.)
│   ├── bus.h         # Pliki nagłówkowe dla autobusu
│   ├── dyspozytor.h  # Pliki nagłówkowe dyspozytora
│   ├── kasa.h        # Pliki nagłówkowe kasy 
│   └── pasazer.h     # Pliki nagłówkowe pasazera
├── tests/            # Skrypty testowe
│   └── test.sh       # Automatyczny test symulacji
├── Makefile          # Kompilacja projektu
└── README.md         # Ten plik
```

## Docker (Konteneryzacjia)
Projekt realizowałem i testowałem na kontenerze Docker (DEBIAN)
```
PROJEKT_SO/
├── .devcontainer/              # Pliki DOCKERA
│   ├── devcontainer.json   
│   ├── Dockerfile 
│   └── reinstall-cmake.sh
```


## Makefile
```
make        - kompilacja wszystkiego
make run    - uruchomienie (domyslne parametry)
make clean  - czyszczenie
make clean-ipc - czyszczenie zasobow IPC
```

## Przykład użycia sygnałów
'''
# Uruchom symulację
./bin/autobus_main 3 10 3 5000 2 &

# Sprawdź PID dyspozytora
pgrep -f autobus_main

# Wymuś odjazd autobusu z peronu
kill -SIGUSR1 <PID>

# Zamknij dworzec (pasażerowie w trasie dojadą do celu)
kill -SIGUSR2 <PID>
'''

## Hierarchia procesów
'''
autobus_main (dyspozytor) ─── PID główny
    │
    ├── kasa ─────────────── proces wielowątkowy
    │   ├── wątek KASA 1
    │   ├── wątek KASA 2
    │   └── ... (K wątków)
    │
    ├── autobus (BUS 1) ──── osobny proces
    ├── autobus (BUS 2)
    └── ... (N autobusów)
    │
    └── pasazer (generator) ─ tworzy pasażerów dynamicznie
        ├── pasazer (PAS 0) ─ normal/dziecko/rodzic
        ├── pasazer (PAS 1)
        └── ... (tworzeni co 800-2000ms)
'''
## Użyte funkcje systemowe
'''
# Procesy

-fork(), execl(), wait(), waitpid(), exit()

# Wątki

-pthread_create(), pthread_join(), pthread_mutex_lock/unlock(), pthread_cond_wait/broadcast()

# Sygnały

-sigaction(), kill(), sigemptyset()

# Semafory SysV

-ftok(), semget(), semctl(), semop()

# Pamięć dzielona SysV

-shmget(), shmat(), shmdt(), shmctl()

# Kolejki komunikatów SysV

-msgget(), msgsnd(), msgrcv(), msgctl()

# Pliki

-creat(), open(), write(), close()
'''

## Konfigurowalne parametry symulacji

# Parametry uruchomieniowe

| Parametr | Domyślna wartość | Opis | Link |
|----------|------------------|------|------|
| `N` | 3 | Liczba autobusów | [src/main.c#L9](https://github.com/JaneckiGit/ProjektSO/blob/main/src/main.c#L9) |
| `P` | 10 | Pojemność autobusu (miejsca normalne) | [src/main.c#L10](https://github.com/JaneckiGit/ProjektSO/blob/main/src/main.c#L10) |
| `R` | 3 | Miejsca na rowery | [src/main.c#L11](https://github.com/JaneckiGit/ProjektSO/blob/main/src/main.c#L11) |
| `T` | 5000 | Czas postoju na peronie [ms] | [src/main.c#L12](https://github.com/JaneckiGit/ProjektSO/blob/main/src/main.c#L12) |
| `K` | 1 (DEFAULT_K) | Liczba kas biletowych | [src/main.c#L13](https://github.com/JaneckiGit/ProjektSO/blob/main/src/main.c#L13) |

# Stałe konfiguracyjne

| Stała | Wartość | Opis | Link |
|-------|---------|------|------|
| `MAX_BUSES` | 50 | Maksymalna liczba autobusów | [include/common.h#L25](https://github.com/JaneckiGit/ProjektSO/blob/main/include/common.h#L25) |
| `MAX_CAPACITY` | 200 | Maksymalna pojemność autobusu | [include/common.h#L26](https://github.com/JaneckiGit/ProjektSO/blob/main/include/common.h#L26) |
| `MAX_REGISTERED` | 1000 | Maks. zarejestrowanych pasażerów | [include/common.h#L27](https://github.com/JaneckiGit/ProjektSO/blob/main/include/common.h#L27) |
| `MAX_CZEKAJACE_DZIECI` | 100 | Maks. dzieci czekających | [include/common.h#L28](https://github.com/JaneckiGit/ProjektSO/blob/main/include/common.h#L28) |
| `MAX_KASY` | 10 | Maksymalna liczba kas | [include/common.h#L29](https://github.com/JaneckiGit/ProjektSO/blob/main/include/common.h#L29) |
| `DEFAULT_K` | 1 | Domyślna liczba kas | [include/common.h#L30](https://github.com/JaneckiGit/ProjektSO/blob/main/include/common.h#L30) |

# Parametry autobusu

| Parametr | Wartość | Opis | Link |
|----------|---------|------|------|
| `czas_trasy_Ti` | losuj(15000, 30000) | Czas trasy [ms] | [src/bus.c#L38](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L38) |
| `czas_dojazdu` (1. kurs) | losuj(1000, 2000) | Dojazd na pętlę [ms] | [src/bus.c#L81](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L81) |
| `czas_dojazdu` (kolejne) | losuj(8000, 15000) | Dojazd na pętlę [ms] | [src/bus.c#L83](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L83) |

# Parametry pasażerów

| Parametr | Wartość | Opis | Link |
|----------|---------|------|------|
| Wiek dorosłego | losuj(9, 80) | Zakres wieku normal | [src/pasazer.c#L152](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L152) |
| Szansa na VIP | 1% | Prawdop. VIP | [src/pasazer.c#L153](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L153) |
| Szansa na rower | 25% | Prawdop. rowerzysty | [src/pasazer.c#L154](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L154) |
| Wiek dziecka | losuj(1, 7) | Zakres wieku dziecka | [src/pasazer.c#L196](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L196) |
| Wiek rodzica | losuj(18, 80) | Zakres wieku opiekuna | [src/pasazer.c#L265](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L265) |
| Interwał generatora | losuj(800, 2000) | Czas między pasażerami [ms] | [src/pasazer.c#L353](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L353) |

# Parametry kasy

| Parametr | Wartość | Opis | Link |
|----------|---------|------|------|
| Czas obsługi | losuj(200, 500) | Czas obsługi pasażera [ms] | [src/kasa.c#L52](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L52) |

#Prawdopodobieństwa typów pasażerów

| Typ | Warunek | Prawdopodobieństwo | Link |
|-----|---------|-------------------|------|
| Dziecko | `los <= 15` | 15% | [src/pasazer.c#L341](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L341) |
| Rodzic | `los <= 55 && dzieci > 0` | 40% | [src/pasazer.c#L343](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L343) |
| Normal | reszta | ~45% | [src/pasazer.c#L346](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L346) |
