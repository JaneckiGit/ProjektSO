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
| `fork()` | `src/dyspozytor.c:178,193,215` `src/pasazer.c:338` |
| `execl()` | `src/dyspozytor.c:181,203,218` `src/pasazer.c:344-346` |
| `wait()` | `src/dyspozytor.c:81` |
| `waitpid()` | `src/dyspozytor.c:38,268` |
| `exit()` | wszystkie pliki źródłowe |

### Wątki
| Funkcja | Lokalizacja |
|---------|-------------|
| `pthread_create()` | `src/pasazer.c:271` |
| `pthread_join()` | `src/pasazer.c:303` |
| `pthread_mutex_lock/unlock()` | `src/pasazer.c:17,20,301,304` |
| `pthread_cond_wait/signal()` | `src/pasazer.c:19,302` |

### Sygnały
| Funkcja | Lokalizacja |
|---------|-------------|
| `sigaction()` | `src/dyspozytor.c:115-126` `src/bus.c:34-38` `src/kasa.c:28-31` |
| `kill()` | `src/dyspozytor.c:63-76,236,248` |
| `sigemptyset()` | `src/dyspozytor.c:114,125` `src/bus.c:37` |

### Semafory SysV
| Funkcja | Lokalizacja |
|---------|-------------|
| `ftok()` | `src/dyspozytor.c:129-133` `src/utils.c:79-83` |
| `semget()` | `src/dyspozytor.c:140` `src/utils.c:90` |
| `semctl()` | `src/dyspozytor.c:44-48,53` |
| `semop()` | `src/bus.c:109-111,230-232` `src/kasa.c:61,67` `src/pasazer.c:57,58` |

### Pamięć dzielona SysV
| Funkcja | Lokalizacja |
|---------|-------------|
| `shmget()` | `src/dyspozytor.c:141` `src/utils.c:91` |
| `shmat()` | `src/dyspozytor.c:150,233,255,276` `src/bus.c:45` `src/kasa.c:40` |
| `shmdt()` | `src/dyspozytor.c:167,238,262,287` `src/bus.c:287` `src/kasa.c:87` |
| `shmctl()` | `src/dyspozytor.c:54` |

### Kolejki komunikatów SysV
| Funkcja | Lokalizacja |
|---------|-------------|
| `msgget()` | `src/dyspozytor.c:142-144` `src/utils.c:92-94` |
| `msgsnd()` | `src/bus.c:147,210,224` `src/kasa.c:74` `src/pasazer.c:43` |
| `msgrcv()` | `src/bus.c:131-134` `src/kasa.c:52` `src/pasazer.c:118,165` |
| `msgctl()` | `src/dyspozytor.c:55-57` |

### Pliki
| Funkcja | Lokalizacja |
|---------|-------------|
| `creat()` | `src/dyspozytor.c:169` |
| `open()` | `src/dyspozytor.c:84` `src/utils.c:55` |
| `write()` | `src/dyspozytor.c:90-98` `src/utils.c:47,59` |
| `close()` | `src/dyspozytor.c:100` `src/utils.c:60` |

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
| `N` | 5 | Liczba autobusów | `src/main.c:9` |
| `P` | 10 | Pojemność autobusu (miejsca normalne) | `src/main.c:10` |
| `R` | 3 | Miejsca na rowery | `src/main.c:11` |
| `T` | 5000 | Czas postoju na peronie [ms] | `src/main.c:12` |
| `K` | 1 | Liczba kas biletowych | `src/main.c:13` |

### Stałe konfiguracyjne

| Stała | Wartość | Opis | Lokalizacja |
|-------|---------|------|-------------|
| `MAX_BUSES` | 50 | Maksymalna liczba autobusów | `include/common.h:25` |
| `MAX_CAPACITY` | 200 | Maksymalna pojemność autobusu | `include/common.h:26` |
| `MAX_REGISTERED` | 100000 | Maks. zarejestrowanych pasażerów | `include/common.h:27` |
| `MAX_KASY` | 10 | Maksymalna liczba kas | `include/common.h:28` |

### Parametry autobusu

| Parametr | Wartość | Opis | Lokalizacja |
|----------|---------|------|-------------|
| `czas_trasy_Ti` | losuj(15000, 30000) | Czas trasy [ms] | `src/bus.c:29` |
| `czas_dojazdu` (1. kurs) | losuj(1000, 2000) | Dojazd na pętlę [ms] | `src/bus.c:72` |
| `czas_dojazdu` (kolejne) | losuj(8000, 15000) | Dojazd na pętlę [ms] | `src/bus.c:72` |

### Parametry pasażerów

| Parametr | Wartość | Opis | Lokalizacja |
|----------|---------|------|-------------|
| Wiek dorosłego | losuj(9, 80) | Zakres wieku pasażera | `src/pasazer.c:212` |
| Szansa na VIP | 1% | Prawdopodobieństwo VIP | `src/pasazer.c:213` |
| Szansa na rower | 25% | Prawdopodobieństwo rowerzysty | `src/pasazer.c:214` |
| Wiek dziecka | losuj(1, 7) | Zakres wieku dziecka | `src/pasazer.c:252` |
| Wiek rodzica | losuj(18, 80) | Zakres wieku opiekuna | `src/pasazer.c:249` |
| Interwał generatora | losuj(800, 2000) | Czas między pasażerami [ms] | `src/pasazer.c:351` |

### Parametry kasy

| Parametr | Wartość | Opis | Lokalizacja |
|----------|---------|------|-------------|
| Czas obsługi | losuj(200, 500) | Czas obsługi pasażera [ms] | `src/kasa.c:58` |

### Prawdopodobieństwa typów pasażerów

| Typ | Prawdopodobieństwo | Lokalizacja |
|-----|-------------------|-------------|
| Rodzic z dzieckiem | 20% | `src/pasazer.c:343` |
| Zwykły pasażer | 80% | `src/pasazer.c:345` |
