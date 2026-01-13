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
| `fork()` | [dyspozytor.c:192](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L192), [dyspozytor.c:205](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L205), [dyspozytor.c:224](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L224), [pasazer.c:337](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L337) |
| `execl()` | [dyspozytor.c:197](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L197), [dyspozytor.c:216](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L216), [dyspozytor.c:228](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L228), [pasazer.c:344](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L344), [pasazer.c:346](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L346) |
| `wait()` | [dyspozytor.c:79](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L79) |
| `waitpid()` | [dyspozytor.c:36](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L36), [dyspozytor.c:280](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L280) |
| `exit()` | wszystkie pliki źródłowe |

### Wątki
| Funkcja | Lokalizacja |
|---------|-------------|
| `pthread_create()` | [pasazer.c:281](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L281) |
| `pthread_join()` | [pasazer.c:305](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L305) |
| `pthread_mutex_lock()` | [pasazer.c:19](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L19), [pasazer.c:301](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L301) |
| `pthread_mutex_unlock()` | [pasazer.c:24](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L24), [pasazer.c:304](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L304) |
| `pthread_cond_wait()` | [pasazer.c:22](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L22) |
| `pthread_cond_signal()` | [pasazer.c:303](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L303) |

### Sygnały
| Funkcja | Lokalizacja |
|---------|-------------|
| `sigaction()` | [dyspozytor.c:119-122](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L119-L122), [dyspozytor.c:130](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L130), [bus.c:37-39](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L37-L39), [kasa.c:27-28](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L27-L28) |
| `kill()` | [dyspozytor.c:64](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L64), [dyspozytor.c:69](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L69), [dyspozytor.c:75](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L75), [dyspozytor.c:243](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L243), [dyspozytor.c:263](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L263) |
| `sigemptyset()` | [dyspozytor.c:117](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L117), [dyspozytor.c:129](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L129), [bus.c:36](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L36), [kasa.c:25](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L25) |

### Semafory SysV
| Funkcja | Lokalizacja |
|---------|-------------|
| `ftok()` | [dyspozytor.c:133-137](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L133-L137), [utils.c:80-84](https://github.com/JaneckiGit/ProjektSO/blob/main/src/utils.c#L80-L84) |
| `semget()` | [dyspozytor.c:144](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L144), [utils.c:91](https://github.com/JaneckiGit/ProjektSO/blob/main/src/utils.c#L91) |
| `semctl()` | [dyspozytor.c:44-48](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L44-L48), [dyspozytor.c:53](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L53) |
| `semop()` | [bus.c:92](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L92), [bus.c:112](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L112), [bus.c:238-239](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L238-L239), [kasa.c:61](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L61), [kasa.c:69](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L69), [pasazer.c:72](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L72), [pasazer.c:102](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L102), [utils.c:29](https://github.com/JaneckiGit/ProjektSO/blob/main/src/utils.c#L29) |

### Pamięć dzielona SysV
| Funkcja | Lokalizacja |
|---------|-------------|
| `shmget()` | [dyspozytor.c:145](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L145), [utils.c:92](https://github.com/JaneckiGit/ProjektSO/blob/main/src/utils.c#L92) |
| `shmat()` | [dyspozytor.c:158](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L158), [dyspozytor.c:237](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L237), [dyspozytor.c:256](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L256), [dyspozytor.c:269](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L269), [bus.c:45](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L45), [kasa.c:34](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L34), [pasazer.c:204](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L204), [pasazer.c:240](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L240) |
| `shmdt()` | [dyspozytor.c:175](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L175), [dyspozytor.c:247](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L247), [dyspozytor.c:259](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L259), [bus.c:285](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L285), [kasa.c:88](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L88), [pasazer.c:209](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L209), [pasazer.c:230](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L230) |
| `shmctl()` | [dyspozytor.c:54](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L54) |

### Kolejki komunikatów SysV
| Funkcja | Lokalizacja |
|---------|-------------|
| `msgget()` | [dyspozytor.c:146-148](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L146-L148), [utils.c:93-95](https://github.com/JaneckiGit/ProjektSO/blob/main/src/utils.c#L93-L95) |
| `msgsnd()` | [bus.c:147](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L147), [bus.c:213](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L213), [bus.c:227](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L227), [kasa.c:76](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L76), [pasazer.c:47](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L47), [pasazer.c:173](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L173) |
| `msgrcv()` | [bus.c:134](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L134), [bus.c:136](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L136), [kasa.c:52](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L52), [pasazer.c:120](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L120), [pasazer.c:178](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L178) |
| `msgctl()` | [dyspozytor.c:55-57](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L55-L57) |

### Pliki
| Funkcja | Lokalizacja |
|---------|-------------|
| `creat()` | [dyspozytor.c:177](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L177) |
| `open()` | [dyspozytor.c:83](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L83), [utils.c:56](https://github.com/JaneckiGit/ProjektSO/blob/main/src/utils.c#L56) |
| `write()` | [dyspozytor.c:97](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L97), [dyspozytor.c:102](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L102), [dyspozytor.c:180](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L180), [utils.c:47](https://github.com/JaneckiGit/ProjektSO/blob/main/src/utils.c#L47), [utils.c:58](https://github.com/JaneckiGit/ProjektSO/blob/main/src/utils.c#L58) |
| `close()` | [dyspozytor.c:104](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L104), [dyspozytor.c:181](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L181), [utils.c:59](https://github.com/JaneckiGit/ProjektSO/blob/main/src/utils.c#L59) |

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
| `N` | 5 | Liczba autobusów | [src/main.c:9](https://github.com/JaneckiGit/ProjektSO/blob/main/src/main.c#L9) |
| `P` | 10 | Pojemność autobusu (miejsca normalne) | [src/main.c:10](https://github.com/JaneckiGit/ProjektSO/blob/main/src/main.c#L10) |
| `R` | 3 | Miejsca na rowery | [src/main.c:11](https://github.com/JaneckiGit/ProjektSO/blob/main/src/main.c#L11) |
| `T` | 5000 | Czas postoju na peronie [ms] | [src/main.c:12](https://github.com/JaneckiGit/ProjektSO/blob/main/src/main.c#L12) |
| `K` | 1 | Liczba kas biletowych | [src/main.c:13](https://github.com/JaneckiGit/ProjektSO/blob/main/src/main.c#L13) |

### Stałe konfiguracyjne

| Stała | Wartość | Opis | Lokalizacja |
|-------|---------|------|-------------|
| `MAX_BUSES` | 50 | Maksymalna liczba autobusów | [include/common.h:25](https://github.com/JaneckiGit/ProjektSO/blob/main/include/common.h#L25) |
| `MAX_CAPACITY` | 200 | Maksymalna pojemność autobusu | [include/common.h:26](https://github.com/JaneckiGit/ProjektSO/blob/main/include/common.h#L26) |
| `MAX_REGISTERED` | 100000 | Maks. zarejestrowanych pasażerów | [include/common.h:27](https://github.com/JaneckiGit/ProjektSO/blob/main/include/common.h#L27) |
| `MAX_KASY` | 10 | Maksymalna liczba kas | [include/common.h:28](https://github.com/JaneckiGit/ProjektSO/blob/main/include/common.h#L28) |

### Parametry autobusu

| Parametr | Wartość | Opis | Lokalizacja |
|----------|---------|------|-------------|
| `czas_trasy_Ti` | losuj(15000, 30000) | Czas trasy [ms] | [src/bus.c:29](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L29) |
| `czas_dojazdu` (1. kurs) | losuj(1000, 2000) | Dojazd na pętlę [ms] | [src/bus.c:72](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L72) |
| `czas_dojazdu` (kolejne) | losuj(8000, 15000) | Dojazd na pętlę [ms] | [src/bus.c:72](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L72) |

### Parametry pasażerów

| Parametr | Wartość | Opis | Lokalizacja |
|----------|---------|------|-------------|
| Wiek dorosłego | losuj(9, 80) | Zakres wieku pasażera | [src/pasazer.c:212](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L212) |
| Szansa na VIP | 1% | Prawdopodobieństwo VIP | [src/pasazer.c:213](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L213) |
| Szansa na rower | 25% | Prawdopodobieństwo rowerzysty | [src/pasazer.c:214](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L214) |
| Wiek dziecka | losuj(1, 7) | Zakres wieku dziecka | [src/pasazer.c:252](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L252) |
| Wiek rodzica | losuj(18, 80) | Zakres wieku opiekuna | [src/pasazer.c:249](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L249) |
| Interwał generatora | losuj(800, 2000) | Czas między pasażerami [ms] | [src/pasazer.c:351](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L351) |

### Parametry kasy

| Parametr | Wartość | Opis | Lokalizacja |
|----------|---------|------|-------------|
| Czas obsługi | losuj(200, 500) | Czas obsługi pasażera [ms] | [src/kasa.c:58](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L58) |

### Prawdopodobieństwa typów pasażerów

| Typ | Prawdopodobieństwo | Lokalizacja |
|-----|-------------------|-------------|
| Rodzic z dzieckiem | 20% | [src/pasazer.c:343](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L343) |
| Zwykły pasażer | 80% | [src/pasazer.c:345](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L345) |
