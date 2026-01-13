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
```bash
# Uruchom symulację
./bin/autobus_main 3 10 3 5000 2 &

# Sprawdź PID dyspozytora
pgrep -f autobus_main

# Wymuś odjazd autobusu z peronu
kill -SIGUSR1 <PID>

# Zamknij dworzec (pasażerowie w trasie dojadą do celu)
kill -SIGUSR2 <PID>
```
## Użyte funkcje systemowe

### Procesy
| Funkcja | Lokalizacja |
|---------|-------------|
| `fork()` | [dyspozytor.c#L189](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L189), [dyspozytor.c#L203](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L203), [dyspozytor.c#L220](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L220), [pasazer.c#L337](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L337) |
| `execl()` | [dyspozytor.c#L193](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L193), [dyspozytor.c#L214](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L214), [pasazer.c#L344](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L344) |
| `wait()` | [dyspozytor.c#L80](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L80) |
| `waitpid()` | [dyspozytor.c#L36](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L36) |
| `exit()` | wszystkie pliki źródłowe |

### Wątki
| Funkcja | Lokalizacja |
|---------|-------------|
| `pthread_create()` | [pasazer.c#L271](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L271) |
| `pthread_join()` | [pasazer.c#L303](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L303) |
| `pthread_mutex_lock/unlock()` | [pasazer.c#L17](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L17), [pasazer.c#L20](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L20), [pasazer.c#L301](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L301) |
| `pthread_cond_wait/signal()` | [pasazer.c#L19](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L19), [pasazer.c#L302](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L302) |

### Sygnały
| Funkcja | Lokalizacja |
|---------|-------------|
| `sigaction()` | [dyspozytor.c#L119-L122](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L119-L122), [bus.c#L38-L40](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L38-L40), [kasa.c#L28-L31](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L28-L31) |
| `kill()` | [dyspozytor.c#L63-L76](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L63-L76) |
| `sigemptyset()` | [dyspozytor.c#L117](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L117), [bus.c#L37](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L37) |

### Semafory SysV
| Funkcja | Lokalizacja |
|---------|-------------|
| `ftok()` | [dyspozytor.c#L133-L137](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L133-L137), [utils.c#L78-L82](https://github.com/JaneckiGit/ProjektSO/blob/main/src/utils.c#L78-L82) |
| `semget()` | [dyspozytor.c#L144](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L144), [utils.c#L90](https://github.com/JaneckiGit/ProjektSO/blob/main/src/utils.c#L90) |
| `semctl()` | [dyspozytor.c#L44-L48](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L44-L48), [dyspozytor.c#L53](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L53) |
| `semop()` | [bus.c#L109-L111](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L109-L111), [kasa.c#L61](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L61), [utils.c#L29](https://github.com/JaneckiGit/ProjektSO/blob/main/src/utils.c#L29) |

### Pamięć dzielona SysV
| Funkcja | Lokalizacja |
|---------|-------------|
| `shmget()` | [dyspozytor.c#L145](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L145), [utils.c#L91](https://github.com/JaneckiGit/ProjektSO/blob/main/src/utils.c#L91) |
| `shmat()` | [dyspozytor.c#L157](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L157), [bus.c#L45](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L45), [kasa.c#L40](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L40) |
| `shmdt()` | [dyspozytor.c#L175](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L175), [bus.c#L287](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L287), [kasa.c#L87](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L87) |
| `shmctl()` | [dyspozytor.c#L54](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L54) |

### Kolejki komunikatów SysV
| Funkcja | Lokalizacja |
|---------|-------------|
| `msgget()` | [dyspozytor.c#L146-L148](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L146-L148), [utils.c#L92-L94](https://github.com/JaneckiGit/ProjektSO/blob/main/src/utils.c#L92-L94) |
| `msgsnd()` | [bus.c#L147](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L147), [kasa.c#L74](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L74), [pasazer.c#L43](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L43) |
| `msgrcv()` | [bus.c#L131-L134](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L131-L134), [kasa.c#L52](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L52), [pasazer.c#L118](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L118) |
| `msgctl()` | [dyspozytor.c#L55-L57](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L55-L57) |

### Pliki
| Funkcja | Lokalizacja |
|---------|-------------|
| `creat()` | [dyspozytor.c#L177](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L177) |
| `open()` | [dyspozytor.c#L84](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L84), [utils.c#L55](https://github.com/JaneckiGit/ProjektSO/blob/main/src/utils.c#L55) |
| `write()` | [dyspozytor.c#L95](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L95), [utils.c#L47](https://github.com/JaneckiGit/ProjektSO/blob/main/src/utils.c#L47), [utils.c#L57](https://github.com/JaneckiGit/ProjektSO/blob/main/src/utils.c#L57) |
| `close()` | [dyspozytor.c#L100](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L100), [utils.c#L58](https://github.com/JaneckiGit/ProjektSO/blob/main/src/utils.c#L58) |

## Testy 
```
TEST 1: Obciążeniowy - czy są odmowy przy małej pojemności?
TEST 2: Czy rowerzyści są odmawiani gdy R=2?
TEST 3: SIGUSR1 (N=2 P=10 R=3 T=15000 K=1)
TEST 4: SIGUSR2 (N=3 P=10 R=3 T=5000 K=2)
```
Uruchomienie: `./tests/test.sh [1|2|3|4|all]`

## Konfigurowalne parametry symulacji

### Parametry uruchomieniowe

| Parametr | Domyślna wartość | Opis | Lokalizacja |
|----------|------------------|------|-------------|
| `N` | 5 | Liczba autobusów | [src/main.c#L9](https://github.com/JaneckiGit/ProjektSO/blob/main/src/main.c#L9) |
| `P` | 10 | Pojemność autobusu (miejsca normalne) | [src/main.c#L10](https://github.com/JaneckiGit/ProjektSO/blob/main/src/main.c#L10) |
| `R` | 3 | Miejsca na rowery | [src/main.c#L11](https://github.com/JaneckiGit/ProjektSO/blob/main/src/main.c#L11) |
| `T` | 5000 | Czas postoju na peronie [ms] | [src/main.c#L12](https://github.com/JaneckiGit/ProjektSO/blob/main/src/main.c#L12) |
| `K` | 1 | Liczba kas biletowych | [src/main.c#L13](https://github.com/JaneckiGit/ProjektSO/blob/main/src/main.c#L13) |

### Stałe konfiguracyjne

| Stała | Wartość | Opis | Lokalizacja |
|-------|---------|------|-------------|
| `MAX_BUSES` | 50 | Maksymalna liczba autobusów | [include/common.h#L25](https://github.com/JaneckiGit/ProjektSO/blob/main/include/common.h#L25) |
| `MAX_CAPACITY` | 200 | Maksymalna pojemność autobusu | [include/common.h#L26](https://github.com/JaneckiGit/ProjektSO/blob/main/include/common.h#L26) |
| `MAX_REGISTERED` | 100000 | Maks. zarejestrowanych pasażerów | [include/common.h#L27](https://github.com/JaneckiGit/ProjektSO/blob/main/include/common.h#L27) |
| `MAX_KASY` | 10 | Maksymalna liczba kas | [include/common.h#L28](https://github.com/JaneckiGit/ProjektSO/blob/main/include/common.h#L28) |

### Parametry autobusu

| Parametr | Wartość | Opis | Lokalizacja |
|----------|---------|------|-------------|
| `czas_trasy_Ti` | losuj(15000, 30000) | Czas trasy [ms] | [src/bus.c#L29](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L29) |
| `czas_dojazdu` (1. kurs) | losuj(1000, 2000) | Dojazd na pętlę [ms] | [src/bus.c#L72](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L72) |
| `czas_dojazdu` (kolejne) | losuj(8000, 15000) | Dojazd na pętlę [ms] | [src/bus.c#L72](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L72) |

### Parametry pasażerów

| Parametr | Wartość | Opis | Lokalizacja |
|----------|---------|------|-------------|
| Wiek dorosłego | losuj(9, 80) | Zakres wieku pasażera | [src/pasazer.c#L212](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L212) |
| Szansa na VIP | 1% | Prawdopodobieństwo VIP | [src/pasazer.c#L213](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L213) |
| Szansa na rower | 25% | Prawdopodobieństwo rowerzysty | [src/pasazer.c#L214](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L214) |
| Wiek dziecka | losuj(1, 7) | Zakres wieku dziecka | [src/pasazer.c#L252](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L252) |
| Wiek rodzica | losuj(18, 80) | Zakres wieku opiekuna | [src/pasazer.c#L249](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L249) |
| Interwał generatora | losuj(800, 2000) | Czas między pasażerami [ms] | [src/pasazer.c#L351](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L351) |

### Parametry kasy

| Parametr | Wartość | Opis | Lokalizacja |
|----------|---------|------|-------------|
| Czas obsługi | losuj(200, 500) | Czas obsługi pasażera [ms] | [src/kasa.c#L58](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L58) |

### Prawdopodobieństwa typów pasażerów

| Typ | Prawdopodobieństwo | Lokalizacja |
|-----|-------------------|-------------|
| Rodzic z dzieckiem | 20% | [src/pasazer.c#L343](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L343) |
| Zwykły pasażer | 80% | [src/pasazer.c#L345](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L345) |
