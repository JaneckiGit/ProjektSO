## Mateusz Janecki
Nr. Albumu: 155182
Kierunek: Informatyka
Rok: 2
Semestr: 3
Grupa Projektowa: 1
Przedmiot: Systemy Operacyjne (Projekt)

## Link do GitHub:
https://github.com/JaneckiGit/

## Link do REPOZYTORIUM PROJEKTU
https://github.com/JaneckiGit/ProjektSO/

## Wybór Projektu: 
Temat 12 
Autobus podmiejski. 
Na dworcu stoi autobus o pojemności P pasażerów, w którym jednocześnie można przewieźć R rowerów. Do autobusu są dwa wejścia (każde może pomieścić jedną osobę): jedno dla pasażerów z bagażem podręcznym i drugie dla pasażerów z rowerami. Autobus odjeżdża co określoną ilość czasu T (np.: co 10 minut). W momencie odjazdu kierowca musi dopilnować aby na wejściach nie było żadnego wchodzącego pasażera. Jednocześnie musi dopilnować by liczba wszystkich pasażerów autobusu nie przekroczyła P i liczba pasażerów z rowerami nie była większa niż R. Po odjeździe autobusu na jego miejsce pojawia się natychmiast (jeżeli jest dostępny) nowy pojazd o takiej samej pojemności jak poprzedni. Łączna liczba autobusów wynosi N, każdy o pojemności P z R miejscami na rowery. Pasażerowie w różnym wieku przychodzą na dworzec w losowych momentach czasu. Przed wejściem na przystanek kupują bilet w kasie (K). Istnieje pewna liczba osób VIP (ok. 1% ) – posiadająca wcześniej wykupiony bilet, które nie płacą za bilet i mogą wejść na przystanek i do autobusu z pominięciem kolejki oczekujących. Dzieci w wieku poniżej 8 roku życia mogą wejść do autobusu tylko pod opieką osoby dorosłej (dziecko zajmuje osobne miejsce w autobusie). Kasa rejestruje wszystkie wchodzące osoby (ID procesu/wątku). Każdy pasażer przy wejściu do autobusu okazuje kierowcy ważny bilet – do autobusu nie może wejść osoba bez wykupionego wcześniej biletu. Autobusy przewożą pasażerów do miejsca docelowego i po czasie Ti (wartość losowa, każdy autobus ma inny czas) wracają na dworzec. Na polecenie dyspozytora (sygnał 1) autobus, który w danym momencie stoi na dworcu może odjechać z niepełną liczbą pasażerów. Po otrzymaniu od dyspozytora polecenia (sygnał 2) pasażerowie nie mogą wsiąść do żadnego autobusu - nie mogą wejść na dworzec. Autobusy kończą pracę po rozwiezieniu wszystkich pasażerów. Napisz programy symulujące działanie dyspozytora, kasy, kierowcy i pasażerów. Raport z przebiegu symulacji zapisać w pliku (plikach) tekstowym.

## Struktura projektu
```
ProjektSO/
├── src/              # Pliki źródłowe
│   ├── main.c        # Proces główny - parsowanie argumentów
│   ├── bus.c         # Proces autobusu/kierowcy
│   ├── dyspozytor.c  # Proces dyspozytora - zarządzanie symulacją
│   ├── kasa.c        # Proces kasy biletowej
│   ├── utils.c       # Funkcje pomocnicze (logi, IPC, losowanie)
│   └── pasazer.c     # Procesy pasażerów (normal, rodzic+dziecko, generator)
├── include/          # Pliki nagłówkowe
│   ├── common.h      # Definicje wspólne (struktury, stałe, kolory)
│   ├── bus.h         # Nagłówek autobusu
│   ├── dyspozytor.h  # Nagłówek dyspozytora
│   ├── kasa.h        # Nagłówek kasy 
│   └── pasazer.h     # Nagłówek pasażera
├── tests/            # Skrypty testowe
│   └── test.sh       # Automatyczny test symulacji
├── Makefile          # Kompilacja projektu
├── raport.txt        # Raport z symulacji (generowany)
└── README.md         # Ten plik
```

## Docker (Konteneryzacja)
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

## Wymagane elementy projektu

### 1. Tworzenie i obsługa procesów oraz wątków
| Mechanizm | Opis | Lokalizacja |
|-----------|------|-------------|
| `fork()` + `execl()` | Tworzenie procesów: kasy, autobusy, pasażerowie | [dyspozytor.c:191-228](src/dyspozytor.c#L191-L228), [pasazer.c:341-350](src/pasazer.c#L341-L350) |
| `pthread_create()` | Wątek dziecka towarzyszącego rodzicowi | [pasazer.c:285](src/pasazer.c#L285) |
| `pthread_join()` | Oczekiwanie na zakończenie wątku dziecka | [pasazer.c:309](src/pasazer.c#L309) |
| `wait()`, `waitpid()` | Zbieranie procesów zombie | [dyspozytor.c:36](src/dyspozytor.c#L36), [dyspozytor.c:79](src/dyspozytor.c#L79), [dyspozytor.c:289](src/dyspozytor.c#L289) |

### 2. Mechanizmy synchronizacji (programowanie współbieżne)
| Mechanizm | Opis | Lokalizacja |
|-----------|------|-------------|
| Semafory SysV | SEM_BUS_STOP (peron), SEM_DOOR_NORMAL/ROWER (drzwi), SEM_SHM (pamięć), SEM_LOG (logi) | [common.h:40-45](include/common.h#L40-L45), [bus.c:50-57](src/bus.c#L50-L57) |
| `semop()` | Operacje P/V na semaforach | [bus.c:92](src/bus.c#L92), [bus.c:112](src/bus.c#L112), [kasa.c:72](src/kasa.c#L72) |
| `pthread_mutex` | Synchronizacja wątku rodzica i dziecka | [pasazer.c:273-275](src/pasazer.c#L273-L275), [pasazer.c:305-308](src/pasazer.c#L305-L308) |
| `pthread_cond` | Sygnalizacja zakończenia wątku dziecka | [pasazer.c:21-22](src/pasazer.c#L21-L22), [pasazer.c:307](src/pasazer.c#L307) |

### 3. Dwa różne mechanizmy komunikacji międzyprocesowej
| Mechanizm | Zastosowanie | Lokalizacja |
|-----------|--------------|-------------|
| **Pamięć dzielona** | Współdzielenie stanu symulacji (SharedData) | [common.h:51-80](include/common.h#L51-L80), [dyspozytor.c:158](src/dyspozytor.c#L158) |
| **Kolejki komunikatów** | msg_id (bilety), msg_kasa_id (kasa), msg_odp_id (odpowiedzi) | [bus.c:134-136](src/bus.c#L134-L136), [kasa.c:52](src/kasa.c#L52), [pasazer.c:47](src/pasazer.c#L47) |

### 4. Obsługa sygnałów 
| Sygnał | Opis | Lokalizacja |
|--------|------|-------------|
| `SIGUSR1` | Wymuszony odjazd autobusu z peronu | [dyspozytor.c:25](src/dyspozytor.c#L25), [bus.c:12-13](src/bus.c#L12-L13) |
| `SIGUSR2` | Zamknięcie dworca (graceful shutdown) | [dyspozytor.c:27](src/dyspozytor.c#L27), [dyspozytor.c:258-276](src/dyspozytor.c#L258-L276) |
| `SIGINT/SIGTERM` | Natychmiastowe zakończenie symulacji | [dyspozytor.c:29](src/dyspozytor.c#L29), [bus.c:14](src/bus.c#L14), [kasa.c:12](src/kasa.c#L12) |
| `SIGCHLD` | Automatyczne zbieranie zombie | [dyspozytor.c:33-36](src/dyspozytor.c#L33-L36), [dyspozytor.c:128-131](src/dyspozytor.c#L128-L131) |

### 5. Obsługa błędów i walidacja danych
| Element | Opis | Lokalizacja |
|---------|------|-------------|
| Walidacja parametrów | Sprawdzanie argc, zakresów N/P/R/T/K | [main.c:22-42](src/main.c#L22-L42) |
| Obsługa błędów IPC | Sprawdzanie zwracanych wartości semget/shmget/msgget | [dyspozytor.c:144-152](src/dyspozytor.c#L144-L152) |
| `perror()` | Raportowanie błędów systemowych | [bus.c:47](src/bus.c#L47), [kasa.c:36](src/kasa.c#L36), [pasazer.c:244](src/pasazer.c#L244) |

### 6. Własne moduły (podział na pliki)
| Moduł | Odpowiedzialność |
|-------|------------------|
| [main.c](src/main.c) | Punkt wejścia, parsowanie argumentów, walidacja |
| [dyspozytor.c](src/dyspozytor.c) | Zarządzanie symulacją, obsługa sygnałów, tworzenie IPC, uruchamianie procesów |
| [bus.c](src/bus.c) | Logika autobusu/kierowcy - przyjmowanie pasażerów, kontrola biletów, trasy |
| [kasa.c](src/kasa.c) | Obsługa kasy biletowej - sprzedaż biletów, rejestracja pasażerów |
| [pasazer.c](src/pasazer.c) | Pasażerowie (normal, rodzic+dziecko jako wątki, generator) |
| [utils.c](src/utils.c) | Funkcje pomocnicze (log_print, losuj, msleep, init_ipc_client) |
| [common.h](include/common.h) | Wspólne struktury (SharedData, BiletMsg, KasaRequest/Response), stałe, kolory |

## Użyte funkcje systemowe

### Procesy
| Funkcja | Lokalizacja |
|---------|-------------|
| `fork()` | [dyspozytor.c:191](src/dyspozytor.c#L191), [dyspozytor.c:205](src/dyspozytor.c#L205), [dyspozytor.c:224](src/dyspozytor.c#L224), [pasazer.c:341](src/pasazer.c#L341) |
| `execl()` | [dyspozytor.c:196](src/dyspozytor.c#L196), [dyspozytor.c:216](src/dyspozytor.c#L216), [dyspozytor.c:228](src/dyspozytor.c#L228), [pasazer.c:346](src/pasazer.c#L346), [pasazer.c:348](src/pasazer.c#L348) |
| `wait()` | [dyspozytor.c:79](src/dyspozytor.c#L79) |
| `waitpid()` | [dyspozytor.c:36](src/dyspozytor.c#L36), [dyspozytor.c:289](src/dyspozytor.c#L289) |
| `exit()` | wszystkie pliki źródłowe |

### Wątki
| Funkcja | Lokalizacja |
|---------|-------------|
| `pthread_create()` | [pasazer.c:285](src/pasazer.c#L285) |
| `pthread_join()` | [pasazer.c:309](src/pasazer.c#L309) |
| `pthread_mutex_lock()` | [pasazer.c:18](src/pasazer.c#L18), [pasazer.c:305](src/pasazer.c#L305) |
| `pthread_mutex_unlock()` | [pasazer.c:23](src/pasazer.c#L23), [pasazer.c:308](src/pasazer.c#L308) |
| `pthread_cond_wait()` | [pasazer.c:21](src/pasazer.c#L21) |
| `pthread_cond_signal()` | [pasazer.c:307](src/pasazer.c#L307) |

### Sygnały
| Funkcja | Lokalizacja |
|---------|-------------|
| `sigaction()` | [dyspozytor.c:119-123](src/dyspozytor.c#L119-L123), [dyspozytor.c:128-131](src/dyspozytor.c#L128-L131), [bus.c:37-39](src/bus.c#L37-L39), [kasa.c:27-28](src/kasa.c#L27-L28) |
| `kill()` | [dyspozytor.c:64](src/dyspozytor.c#L64), [dyspozytor.c:69](src/dyspozytor.c#L69), [dyspozytor.c:75](src/dyspozytor.c#L75), [dyspozytor.c:252](src/dyspozytor.c#L252), [dyspozytor.c:272](src/dyspozytor.c#L272) |
| `sigemptyset()` | [dyspozytor.c:117](src/dyspozytor.c#L117), [dyspozytor.c:130](src/dyspozytor.c#L130), [bus.c:36](src/bus.c#L36), [kasa.c:25](src/kasa.c#L25) |
| `signal()` | [dyspozytor.c:109](src/dyspozytor.c#L109), [pasazer.c:321](src/pasazer.c#L321) |

### Semafory SysV
| Funkcja | Lokalizacja |
|---------|-------------|
| `ftok()` | [dyspozytor.c:133-137](src/dyspozytor.c#L133-L137), [utils.c:80-84](src/utils.c#L80-L84) |
| `semget()` | [dyspozytor.c:144](src/dyspozytor.c#L144), [utils.c:91](src/utils.c#L91) |
| `semctl()` | [dyspozytor.c:44-48](src/dyspozytor.c#L44-L48), [dyspozytor.c:53](src/dyspozytor.c#L53) |
| `semop()` | [bus.c:92](src/bus.c#L92), [bus.c:112](src/bus.c#L112), [bus.c:240-241](src/bus.c#L240-L241), [kasa.c:72](src/kasa.c#L72), [kasa.c:78](src/kasa.c#L78), [pasazer.c:72](src/pasazer.c#L72), [pasazer.c:102](src/pasazer.c#L102), [utils.c:29](src/utils.c#L29) |

### Pamięć dzielona SysV
| Funkcja | Lokalizacja |
|---------|-------------|
| `shmget()` | [dyspozytor.c:145](src/dyspozytor.c#L145), [utils.c:92](src/utils.c#L92) |
| `shmat()` | [dyspozytor.c:158](src/dyspozytor.c#L158), [dyspozytor.c:246](src/dyspozytor.c#L246), [dyspozytor.c:265](src/dyspozytor.c#L265), [dyspozytor.c:278](src/dyspozytor.c#L278), [dyspozytor.c:296](src/dyspozytor.c#L296), [bus.c:45](src/bus.c#L45), [kasa.c:34](src/kasa.c#L34), [pasazer.c:204](src/pasazer.c#L204), [pasazer.c:243](src/pasazer.c#L243), [pasazer.c:323](src/pasazer.c#L323) |
| `shmdt()` | [dyspozytor.c:175](src/dyspozytor.c#L175), [dyspozytor.c:256](src/dyspozytor.c#L256), [dyspozytor.c:268](src/dyspozytor.c#L268), [bus.c:286](src/bus.c#L286), [kasa.c:94](src/kasa.c#L94), [pasazer.c:210](src/pasazer.c#L210), [pasazer.c:233](src/pasazer.c#L233), [pasazer.c:313](src/pasazer.c#L313), [pasazer.c:328](src/pasazer.c#L328) |
| `shmctl()` | [dyspozytor.c:54](src/dyspozytor.c#L54) |

### Kolejki komunikatów SysV
| Funkcja | Lokalizacja |
|---------|-------------|
| `msgget()` | [dyspozytor.c:146-148](src/dyspozytor.c#L146-L148), [utils.c:93-95](src/utils.c#L93-L95) |
| `msgsnd()` | [bus.c:147](src/bus.c#L147), [bus.c:213](src/bus.c#L213), [bus.c:227](src/bus.c#L227), [kasa.c:68](src/kasa.c#L68), [kasa.c:83](src/kasa.c#L83), [pasazer.c:47](src/pasazer.c#L47), [pasazer.c:173](src/pasazer.c#L173) |
| `msgrcv()` | [bus.c:134](src/bus.c#L134), [bus.c:136](src/bus.c#L136), [kasa.c:52](src/kasa.c#L52), [pasazer.c:120](src/pasazer.c#L120), [pasazer.c:178](src/pasazer.c#L178) |
| `msgctl()` | [dyspozytor.c:55-57](src/dyspozytor.c#L55-L57) |

### Pliki
| Funkcja | Lokalizacja |
|---------|-------------|
| `creat()` | [dyspozytor.c:177](src/dyspozytor.c#L177) |
| `open()` | [dyspozytor.c:83](src/dyspozytor.c#L83), [utils.c:56](src/utils.c#L56) |
| `write()` | [dyspozytor.c:97](src/dyspozytor.c#L97), [dyspozytor.c:102](src/dyspozytor.c#L102), [dyspozytor.c:180](src/dyspozytor.c#L180), [utils.c:47](src/utils.c#L47), [utils.c:58](src/utils.c#L58) |
| `close()` | [dyspozytor.c:104](src/dyspozytor.c#L104), [dyspozytor.c:181](src/dyspozytor.c#L181), [utils.c:59](src/utils.c#L59) |

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
| `N` | 5 | Liczba autobusów | [main.c:9](src/main.c#L9) |
| `P` | 10 | Pojemność autobusu (miejsca normalne) | [main.c:10](src/main.c#L10) |
| `R` | 3 | Miejsca na rowery | [main.c:11](src/main.c#L11) |
| `T` | 5000 | Czas postoju na peronie [ms] | [main.c:12](src/main.c#L12) |
| `K` | 1 | Liczba kas biletowych | [main.c:13](src/main.c#L13) |

### Stałe konfiguracyjne

| Stała | Wartość | Opis | Lokalizacja |
|-------|---------|------|-------------|
| `MAX_BUSES` | 50 | Maksymalna liczba autobusów | [common.h:25](include/common.h#L25) |
| `MAX_CAPACITY` | 200 | Maksymalna pojemność autobusu | [common.h:26](include/common.h#L26) |
| `MAX_REGISTERED` | 100000 | Maks. zarejestrowanych pasażerów | [common.h:27](include/common.h#L27) |
| `MAX_KASY` | 10 | Maksymalna liczba kas | [common.h:28](include/common.h#L28) |

### Parametry autobusu

| Parametr | Wartość | Opis | Lokalizacja |
|----------|---------|------|-------------|
| `czas_trasy_Ti` | losuj(15000, 30000) | Czas trasy do miejsca docelowego [ms] | [bus.c:29](src/bus.c#L29) |
| `czas_dojazdu` (1. kurs) | losuj(1000, 2000) | Dojazd na dworzec [ms] | [bus.c:72](src/bus.c#L72) |
| `czas_dojazdu` (kolejne) | losuj(8000, 15000) | Powrót na dworzec [ms] | [bus.c:72](src/bus.c#L72) |

### Parametry pasażerów

| Parametr | Wartość | Opis | Lokalizacja |
|----------|---------|------|-------------|
| Wiek dorosłego | losuj(9, 80) | Zakres wieku pasażera | [pasazer.c:215](src/pasazer.c#L215) |
| Szansa na VIP | 1% | Prawdopodobieństwo VIP | [pasazer.c:216](src/pasazer.c#L216) |
| Szansa na rower | 25% | Prawdopodobieństwo rowerzysty | [pasazer.c:217](src/pasazer.c#L217) |
| Wiek dziecka | losuj(1, 7) | Zakres wieku dziecka (<8 lat) | [pasazer.c:255](src/pasazer.c#L255) |
| Wiek rodzica | losuj(18, 80) | Zakres wieku opiekuna | [pasazer.c:252](src/pasazer.c#L252) |
| Interwał generatora | losuj(800, 2000) | Czas między pasażerami [ms] | [pasazer.c:355](src/pasazer.c#L355) |

### Parametry kasy

| Parametr | Wartość | Opis | Lokalizacja |
|----------|---------|------|-------------|
| Czas obsługi | losuj(200, 500) | Czas obsługi pasażera [ms] | [kasa.c:58](src/kasa.c#L58) |

### Prawdopodobieństwa typów pasażerów

| Typ | Prawdopodobieństwo | Lokalizacja |
|-----|-------------------|-------------|
| Rodzic z dzieckiem | 20% | [pasazer.c:347](src/pasazer.c#L347) |
| Zwykły pasażer | 80% | [pasazer.c:349](src/pasazer.c#L349) |
