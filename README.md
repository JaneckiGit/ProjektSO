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
| `fork()` + `execl()` | Tworzenie procesów kas | [dyspozytor.c:208-214](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L208-L214) |
| `fork()` + `execl()` | Tworzenie procesów autobusów | [dyspozytor.c:221-233](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L221-L233) |
| `fork()` + `execl()` | Tworzenie generatora pasażerów | [dyspozytor.c:240-245](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L240-L245) |
| `fork()` + `execl()` | Tworzenie procesów pasażerów | [pasazer.c:483-492](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L483-L492) |
| `pthread_create()` | Wątek dziecka towarzyszącego rodzicowi | [pasazer.c:413](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L413) |
| `pthread_join()` | Oczekiwanie na zakończenie wątku dziecka | [pasazer.c:438](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L438), [pasazer.c:457](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L457) |
| `waitpid()` | Zbieranie procesów zombie | [dyspozytor.c:36](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L36), [dyspozytor.c:80](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L80), [dyspozytor.c:304](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L304) |

### 2. Mechanizmy synchronizacji (programowanie współbieżne)
| Mechanizm | Opis | Lokalizacja |
|-----------|------|-------------|
| Semafory SysV | Inicjalizacja semaforów (DOOR_NORMAL, DOOR_ROWER, BUS_STOP, LOG, SHM) | [dyspozytor.c:44-48](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L44-L48) |
| `semop()` | Zajęcie peronu przez autobus | [bus.c:96](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L96) |
| `semop()` | Zwolnienie peronu | [bus.c:114](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L114), [bus.c:336](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L336) |
| `semop()` | Blokada drzwi (pasażer z rowerem) | [pasazer.c:92](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L92), [pasazer.c:99](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L99) |
| `semop()` | Blokada drzwi (pasażer normalny) | [pasazer.c:196](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L196) |
| `semop()` | Blokada pamięci dzielonej w kasie | [kasa.c:84](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L84) |
| `pthread_mutex` | Inicjalizacja mutexa rodzic-dziecko | [pasazer.c:401](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L401) |
| `pthread_mutex_lock/unlock` | Synchronizacja wątków | [pasazer.c:19](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L19), [pasazer.c:24](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L24), [pasazer.c:434](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L434), [pasazer.c:437](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L437) |
| `pthread_cond_wait` | Dziecko czeka na sygnał od rodzica | [pasazer.c:22](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L22) |
| `pthread_cond_signal` | Rodzic sygnalizuje zakończenie | [pasazer.c:436](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L436), [pasazer.c:455](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L455) |

### 3. Dwa różne mechanizmy komunikacji międzyprocesowej
| Mechanizm | Zastosowanie | Lokalizacja |
|-----------|--------------|-------------|
| **Pamięć dzielona** | Tworzenie segmentu SharedData | [dyspozytor.c:163](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L163) |
| **Pamięć dzielona** | Dołączanie do segmentu | [dyspozytor.c:174](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L174), [bus.c:46](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L46), [kasa.c:35](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L35), [pasazer.c:299](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L299) |
| **Kolejki komunikatów** | Tworzenie kolejek (bilety + kasa) | [dyspozytor.c:164-165](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L164-L165) |
| **Kolejki komunikatów** | Wysyłanie biletu do autobusu | [pasazer.c:46](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L46) |
| **Kolejki komunikatów** | Odbieranie biletów (VIP + zwykły) | [bus.c:164-166](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L164-L166) |
| **Kolejki komunikatów** | Komunikacja pasażer-kasa | [kasa.c:52](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L52), [pasazer.c:264](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L264) |

### 4. Obsługa sygnałów 
| Sygnał | Opis | Lokalizacja |
|--------|------|-------------|
| `SIGUSR1` | Handler dyspozytora (wymuszony odjazd) | [dyspozytor.c:26](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L26) |
| `SIGUSR1` | Wysłanie do autobusu na peronie | [dyspozytor.c:260](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L260) |
| `SIGUSR1` | Handler autobusu | [bus.c:12](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L12) |
| `SIGUSR2` | Handler dyspozytora (zamknięcie dworca) | [dyspozytor.c:27](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L27) |
| `SIGUSR2` | Obsługa zamknięcia dworca | [dyspozytor.c:273-285](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L273-L285) |
| `SIGINT/SIGTERM` | Handler dyspozytora | [dyspozytor.c:29](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L29) |
| `SIGINT/SIGTERM` | Handler autobusu | [bus.c:14](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L14) |
| `SIGINT/SIGTERM` | Handler kasy | [kasa.c:13](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L13) |
| `SIGCHLD` | Handler zbierania zombie | [dyspozytor.c:34-37](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L34-L37) |
| `sigaction()` | Rejestracja handlerów dyspozytora | [dyspozytor.c:137-141](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L137-L141), [dyspozytor.c:149](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L149) |
| `sigaction()` | Rejestracja handlerów autobusu | [bus.c:37-40](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L37-L40) |
| `sigaction()` | Rejestracja handlerów kasy | [kasa.c:27-29](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L27-L29) |
| `kill()` | Wysyłanie sygnałów do procesów | [dyspozytor.c:63](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L63), [dyspozytor.c:68](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L68), [dyspozytor.c:74](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L74), [dyspozytor.c:83-88](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L83-L88) |

### 5. Obsługa błędów i walidacja danych
| Element | Opis | Lokalizacja |
|---------|------|-------------|
| Walidacja parametrów | Sprawdzanie argc, zakresów N/P/R/T/K | [main.c:22-41](https://github.com/JaneckiGit/ProjektSO/blob/main/src/main.c#L22-L41) |
| Obsługa błędów IPC | Sprawdzanie zwracanych wartości semget/shmget/msgget | [dyspozytor.c:162-170](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L162-L170) |
| `perror()` | Błędy autobusu | [bus.c:48](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L48), [bus.c:105](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L105) |
| `perror()` | Błędy kasy | [kasa.c:37](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L37) |
| `perror()` | Błędy pasażera | [pasazer.c:265](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L265), [pasazer.c:300](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L300) |

### 6. Własne moduły (podział na pliki)
| Moduł | Odpowiedzialność |
|-------|------------------|
| [main.c](https://github.com/JaneckiGit/ProjektSO/blob/main/src/main.c) | Punkt wejścia, parsowanie argumentów, walidacja |
| [dyspozytor.c](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c) | Zarządzanie symulacją, obsługa sygnałów, tworzenie IPC, uruchamianie procesów |
| [bus.c](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c) | Logika autobusu/kierowcy - przyjmowanie pasażerów, kontrola biletów, trasy |
| [kasa.c](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c) | Obsługa kasy biletowej - sprzedaż biletów, rejestracja pasażerów |
| [pasazer.c](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c) | Pasażerowie (normal, rodzic+dziecko jako wątki, generator), VIP omija kolejkę |
| [utils.c](https://github.com/JaneckiGit/ProjektSO/blob/main/src/utils.c) | Funkcje pomocnicze (log_print, losuj, init_ipc_client) |
| [common.h](https://github.com/JaneckiGit/ProjektSO/blob/main/include/common.h) | Wspólne struktury (SharedData, BiletMsg, KasaRequest/Response), stałe, kolory |

## Użyte funkcje systemowe

### Procesy
| Funkcja | Lokalizacja |
|---------|-------------|
| `fork()` | [dyspozytor.c:208](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L208) (kasy), [dyspozytor.c:221](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L221) (autobusy), [dyspozytor.c:240](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L240) (generator), [pasazer.c:483](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L483) (pasażerowie) |
| `execl()` | [dyspozytor.c:213](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L213) (kasa), [dyspozytor.c:232](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L232) (autobus), [dyspozytor.c:244](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L244) (generator), [pasazer.c:490](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L490), [pasazer.c:492](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L492) (pasażerowie) |
| `waitpid()` | [dyspozytor.c:36](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L36), [dyspozytor.c:80](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L80), [dyspozytor.c:91](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L91), [dyspozytor.c:304](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L304), [dyspozytor.c:344](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L344) |
| `exit()` | [bus.c:49](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L49), [bus.c:384](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L384), [kasa.c:38](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L38), [kasa.c:119](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L119), [pasazer.c:131](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L131), [pasazer.c:183](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L183) |

### Wątki
| Funkcja | Lokalizacja |
|---------|-------------|
| `pthread_create()` | [pasazer.c:413](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L413) |
| `pthread_join()` | [pasazer.c:438](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L438), [pasazer.c:457](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L457) |
| `pthread_mutex_lock()` | [pasazer.c:19](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L19), [pasazer.c:434](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L434), [pasazer.c:453](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L453) |
| `pthread_mutex_unlock()` | [pasazer.c:24](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L24), [pasazer.c:437](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L437), [pasazer.c:456](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L456) |
| `pthread_cond_wait()` | [pasazer.c:22](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L22) |
| `pthread_cond_signal()` | [pasazer.c:436](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L436), [pasazer.c:455](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L455) |
| `pthread_mutex_destroy()` | [pasazer.c:440](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L440), [pasazer.c:459](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L459) |
| `pthread_cond_destroy()` | [pasazer.c:441](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L441), [pasazer.c:460](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L460) |
| `PTHREAD_MUTEX_INITIALIZER` | [pasazer.c:401](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L401) |
| `PTHREAD_COND_INITIALIZER` | [pasazer.c:402](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L402) |

### Sygnały
| Funkcja | Lokalizacja |
|---------|-------------|
| `sigaction()` | [dyspozytor.c:137-141](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L137-L141), [dyspozytor.c:149](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L149), [bus.c:37-40](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L37-L40), [kasa.c:27-29](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L27-L29) |
| `sigemptyset()` | [dyspozytor.c:135](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L135), [dyspozytor.c:148](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L148), [bus.c:36](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L36), [kasa.c:25](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L25) |
| `signal()` | [dyspozytor.c:127](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L127), [pasazer.c:467](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L467) |
| `kill()` | [dyspozytor.c:63](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L63), [dyspozytor.c:68](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L68), [dyspozytor.c:74](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L74), [dyspozytor.c:83-88](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L83-L88), [dyspozytor.c:260](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L260), [dyspozytor.c:280](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L280), [dyspozytor.c:285](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L285) |

### Semafory SysV
| Funkcja | Lokalizacja |
|---------|-------------|
| `ftok()` | [dyspozytor.c:152-155](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L152-L155), [utils.c:78-81](https://github.com/JaneckiGit/ProjektSO/blob/main/src/utils.c#L78-L81) |
| `semget()` | [dyspozytor.c:162](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L162), [utils.c:88](https://github.com/JaneckiGit/ProjektSO/blob/main/src/utils.c#L88) |
| `semctl()` | [dyspozytor.c:44-48](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L44-L48), [dyspozytor.c:53](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L53) |
| `semop()` | [bus.c:96](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L96), [bus.c:114](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L114), [bus.c:122](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L122), [bus.c:317-318](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L317-L318), [bus.c:334-336](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L334-L336), [kasa.c:84](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L84), [kasa.c:95](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L95), [pasazer.c:92](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L92), [pasazer.c:99](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L99), [pasazer.c:196](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L196), [utils.c:29](https://github.com/JaneckiGit/ProjektSO/blob/main/src/utils.c#L29), [utils.c:65](https://github.com/JaneckiGit/ProjektSO/blob/main/src/utils.c#L65) |

### Pamięć dzielona SysV
| Funkcja | Lokalizacja |
|---------|-------------|
| `shmget()` | [dyspozytor.c:163](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L163), [utils.c:89](https://github.com/JaneckiGit/ProjektSO/blob/main/src/utils.c#L89) |
| `shmat()` | [dyspozytor.c:174](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L174), [dyspozytor.c:253](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L253), [dyspozytor.c:273](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L273), [dyspozytor.c:291](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L291), [dyspozytor.c:310](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L310), [dyspozytor.c:335](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L335), [dyspozytor.c:350](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L350), [bus.c:46](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L46), [kasa.c:35](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L35), [pasazer.c:299](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L299), [pasazer.c:360](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L360), [pasazer.c:471](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L471) |
| `shmdt()` | [dyspozytor.c:191](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L191), [dyspozytor.c:264](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L264), [dyspozytor.c:276](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L276), [dyspozytor.c:299](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L299), [dyspozytor.c:302](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L302), [dyspozytor.c:322](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L322), [dyspozytor.c:338](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L338), [dyspozytor.c:368](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L368), [bus.c:383](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L383), [kasa.c:118](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L118), [pasazer.c:131](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L131), [pasazer.c:183](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L183) |
| `shmctl()` | [dyspozytor.c:54](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L54) |

### Kolejki komunikatów SysV
| Funkcja | Lokalizacja |
|---------|-------------|
| `msgget()` | [dyspozytor.c:164-165](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L164-L165), [utils.c:90-91](https://github.com/JaneckiGit/ProjektSO/blob/main/src/utils.c#L90-L91) |
| `msgsnd()` | [bus.c:178](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L178), [bus.c:236](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L236), [bus.c:255](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L255), [bus.c:302](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L302), [bus.c:306](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L306), [kasa.c:62](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L62), [kasa.c:79](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L79), [kasa.c:97](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L97), [kasa.c:114](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L114), [pasazer.c:46](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L46), [pasazer.c:264](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L264) |
| `msgrcv()` | [bus.c:164](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L164), [bus.c:166](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L166), [bus.c:300](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L300), [bus.c:304](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L304), [bus.c:310-311](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L310-L311), [kasa.c:52](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L52), [kasa.c:107](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L107), [pasazer.c:70](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L70), [pasazer.c:120](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L120), [pasazer.c:173](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L173), [pasazer.c:271](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L271) |
| `msgctl()` | [dyspozytor.c:55-56](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L55-L56) |

### Pliki
| Funkcja | Lokalizacja |
|---------|-------------|
| `creat()` | [dyspozytor.c:193](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L193) |
| `open()` | [dyspozytor.c:95](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L95), [utils.c:58](https://github.com/JaneckiGit/ProjektSO/blob/main/src/utils.c#L58) |
| `write()` | [dyspozytor.c:115](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L115), [dyspozytor.c:120](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L120), [dyspozytor.c:196](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L196), [utils.c:49](https://github.com/JaneckiGit/ProjektSO/blob/main/src/utils.c#L49), [utils.c:60](https://github.com/JaneckiGit/ProjektSO/blob/main/src/utils.c#L60) |
| `close()` | [dyspozytor.c:122](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L122), [dyspozytor.c:197](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L197), [utils.c:61](https://github.com/JaneckiGit/ProjektSO/blob/main/src/utils.c#L61) |

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
| `N` | 5 | Liczba autobusów | [main.c:9](https://github.com/JaneckiGit/ProjektSO/blob/main/src/main.c#L9) |
| `P` | 10 | Pojemność autobusu (miejsca normalne) | [main.c:10](https://github.com/JaneckiGit/ProjektSO/blob/main/src/main.c#L10) |
| `R` | 3 | Miejsca na rowery | [main.c:11](https://github.com/JaneckiGit/ProjektSO/blob/main/src/main.c#L11) |
| `T` | 5000 | Czas postoju na peronie [ms] | [main.c:12](https://github.com/JaneckiGit/ProjektSO/blob/main/src/main.c#L12) |
| `K` | 3 | Liczba kas biletowych | [main.c:13](https://github.com/JaneckiGit/ProjektSO/blob/main/src/main.c#L13) |

### Parametry autobusu

| Parametr | Wartość | Opis | Lokalizacja |
|----------|---------|------|-------------|
| `czas_trasy_Ti` | losuj(15000, 30000) | Czas trasy do miejsca docelowego [ms] | [bus.c:29](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L29) |
| `czas_dojazdu` (1. kurs) | losuj(1000, 2000) | Dojazd na dworzec [ms] | [bus.c:73](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L73) |
| `czas_dojazdu` (kolejne) | losuj(8000, 15000) | Powrót na dworzec [ms] | [bus.c:73](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L73) |

### Parametry pasażerów

| Parametr | Wartość | Opis | Lokalizacja |
|----------|---------|------|-------------|
| Wiek dorosłego | losuj(9, 80) | Zakres wieku pasażera | [pasazer.c:307](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L307) |
| Szansa na rower | 25% | Prawdopodobieństwo rowerzysty | [pasazer.c:308](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L308) |
| Szansa na VIP | 1% (tylko bez roweru) | Prawdopodobieństwo VIP | [pasazer.c:310](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L310) |
| Wiek dziecka | losuj(1, 7) | Zakres wieku dziecka (<8 lat) | [pasazer.c:372](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L372) |
| Wiek rodzica | losuj(18, 80) | Zakres wieku opiekuna | [pasazer.c:369](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L369) |
| Interwał generatora | losuj(1, 2) s | Czas między pasażerami | [pasazer.c:499](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L499) |

### Prawdopodobieństwa typów pasażerów

| Typ | Prawdopodobieństwo | Lokalizacja |
|-----|-------------------|-------------|
| Rodzic z dzieckiem | 20% | [pasazer.c:489](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L489) |
| Zwykły pasażer | 80% | [pasazer.c:492](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L492) |

## Specjalne zachowania

### VIP (pasazer.c:150-189)
- VIP **nie może mieć roweru** (linia 309-311)
- VIP **omija semafor drzwi** - wchodzi bez kolejki (linia 150)
- VIP ma **priorytet w kolejce komunikatów** - mtype = bus_pid (linia 34)
- VIP **omija kasę** - ma wcześniej wykupiony bilet (linia 287)

### Pasażer w trasie (bus.c:210)
- Licznik `pasazerow_w_trasie` jest inkrementowany **natychmiast przy wsiadaniu** (nie przy odjeździe)
- Zapewnia to poprawne śledzenie pasażerów nawet przy nagłym przerwaniu symulacji
