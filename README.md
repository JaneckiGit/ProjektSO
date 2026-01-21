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

---

## Najważniejsze funkcje projektu

### Główne funkcje procesów
| Funkcja | Opis | Lokalizacja |
|---------|------|-------------|
| [`proces_dyspozytor()`](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L118) | Główna funkcja dyspozytora - zarządzanie symulacją, IPC, sygnały | [dyspozytor.c:118](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L118) |
| [`proces_autobus()`](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L25) | Główna funkcja autobusu - pętla kursów, zbieranie pasażerów | [bus.c:25](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L25) |
| [`proces_kasa()`](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L17) | Główna funkcja kasy - sprzedaż biletów | [kasa.c:17](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L17) |
| [`proces_pasazer()`](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L295) | Proces zwykłego pasażera | [pasazer.c:295](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L295) |
| [`proces_rodzic_z_dzieckiem()`](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L362) | Proces rodzica z dzieckiem (wątki) | [pasazer.c:362](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L362) |
| [`proces_generator()`](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L458) | Generator nowych pasażerów | [pasazer.c:458](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L458) |

### Funkcje pomocnicze
| Funkcja | Opis | Lokalizacja |
|---------|------|-------------|
| [`handler_dyspozytor()`](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L24) | Handler sygnałów dyspozytora | [dyspozytor.c:24](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L24) |
| [`handler_bus()`](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L12) | Handler sygnałów autobusu | [bus.c:12](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L12) |
| [`handler_kasa()`](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L11) | Handler sygnałów kasy | [kasa.c:11](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L11) |
| [`czekaj_na_autobus()`](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L58) | Pętla oczekiwania pasażera na autobus | [pasazer.c:58](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L58) |
| [`kup_bilet()`](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L263) | Komunikacja pasażer-kasa | [pasazer.c:263](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L263) |
| [`wyslij_bilet()`](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L28) | Wysyłanie biletu do autobusu | [pasazer.c:28](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L28) |
| [`watek_dziecko()`](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L12) | Funkcja wątku dziecka | [pasazer.c:12](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L12) |
| [`init_semafory()`](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L40) | Inicjalizacja semaforów | [dyspozytor.c:40](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L40) |
| [`cleanup_ipc()`](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L51) | Sprzątanie zasobów IPC | [dyspozytor.c:51](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L51) |
| [`shutdown_children()`](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L59) | Zamykanie procesów potomnych | [dyspozytor.c:59](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L59) |
| [`log_print()`](https://github.com/JaneckiGit/ProjektSO/blob/main/src/utils.c#L23) | Logowanie z kolorami i do pliku | [utils.c:23](https://github.com/JaneckiGit/ProjektSO/blob/main/src/utils.c#L23) |
| [`init_ipc_client()`](https://github.com/JaneckiGit/ProjektSO/blob/main/src/utils.c#L70) | Inicjalizacja IPC dla procesów potomnych | [utils.c:70](https://github.com/JaneckiGit/ProjektSO/blob/main/src/utils.c#L70) |

---

## Wymagane elementy projektu

### 1. Tworzenie i obsługa procesów oraz wątków
| Mechanizm | Opis | Lokalizacja |
|-----------|------|-------------|
| `fork()` + `execl()` | Tworzenie procesów kas | [dyspozytor.c:199-212](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L199-L212) |
| `fork()` + `execl()` | Tworzenie procesów autobusów | [dyspozytor.c:214-231](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L214-L231) |
| `fork()` + `execl()` | Tworzenie generatora pasażerów | [dyspozytor.c:235-244](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L235-L244) |
| `fork()` + `execl()` | Tworzenie procesów pasażerów | [pasazer.c:478-491](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L478-L491) |
| `pthread_create()` | Wątek dziecka towarzyszącego rodzicowi | [pasazer.c:411](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L411) |
| `pthread_join()` | Oczekiwanie na zakończenie wątku dziecka | [pasazer.c:437](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L437), [pasazer.c:452](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L452) |
| `waitpid()` | Zbieranie procesów zombie | [dyspozytor.c:35](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L35), [dyspozytor.c:78](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L78) |

### 2. Mechanizmy synchronizacji (programowanie współbieżne)
| Mechanizm | Opis | Lokalizacja |
|-----------|------|-------------|
| Semafory SysV | Inicjalizacja semaforów (DOOR_NORMAL, DOOR_ROWER, BUS_STOP, LOG, SHM) | [dyspozytor.c:43-48](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L43-L48) |
| `semop()` | Zajęcie peronu przez autobus | [bus.c:97-106](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L97-L106) |
| `semop()` | Blokada drzwi (pasażer z rowerem - atomowa) | [pasazer.c:92](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L92) |
| `semop()` | Blokada drzwi (pasażer normalny) | [pasazer.c:199](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L199) |
| `semop()` | Blokada pamięci dzielonej w kasie | [kasa.c:93-96](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L93-L96) |
| `semop()` | Zamknięcie drzwi przed odjazdem (atomowe) | [bus.c:280-283](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L280-L283) |
| `pthread_mutex` | Inicjalizacja mutexa rodzic-dziecko | [pasazer.c:398](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L398) |
| `pthread_mutex_lock/unlock` | Synchronizacja wątków | [pasazer.c:21](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L21), [pasazer.c:25](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L25) |
| `pthread_cond_wait` | Dziecko czeka na sygnał od rodzica | [pasazer.c:24](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L24) |
| `pthread_cond_signal` | Rodzic sygnalizuje zakończenie | [pasazer.c:434](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L434), [pasazer.c:449](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L449) |

### 3. Dwa różne mechanizmy komunikacji międzyprocesowej
| Mechanizm | Zastosowanie | Lokalizacja |
|-----------|--------------|-------------|
| **Pamięć dzielona** | Tworzenie segmentu SharedData | [dyspozytor.c:160](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L160) |
| **Pamięć dzielona** | Dołączanie do segmentu (dyspozytor) | [dyspozytor.c:171](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L171) |
| **Pamięć dzielona** | Dołączanie do segmentu (autobus) | [bus.c:46](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L46) |
| **Pamięć dzielona** | Dołączanie do segmentu (kasa) | [kasa.c:37](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L37) |
| **Pamięć dzielona** | Dołączanie do segmentu (pasażer) | [pasazer.c:300](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L300) |
| **Kolejki komunikatów** | Tworzenie kolejek (bilety + kasa) | [dyspozytor.c:161-162](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L161-L162) |
| **Kolejki komunikatów** | Wysyłanie biletu do autobusu | [pasazer.c:45](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L45) |
| **Kolejki komunikatów** | Odbieranie biletów (VIP + zwykły) | [bus.c:145-148](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L145-L148) |
| **Kolejki komunikatów** | Komunikacja pasażer-kasa (żądanie) | [pasazer.c:282](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L282) |
| **Kolejki komunikatów** | Komunikacja pasażer-kasa (odpowiedź) | [kasa.c:53](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L53) |

### 4. Obsługa sygnałów 
| Sygnał | Opis | Lokalizacja |
|--------|------|-------------|
| `SIGUSR1` | Handler dyspozytora (wymuszony odjazd) | [dyspozytor.c:25](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L25) |
| `SIGUSR1` | Obsługa w pętli głównej dyspozytora | [dyspozytor.c:248-262](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L248-L262) |
| `SIGUSR1` | Handler autobusu | [bus.c:13](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L13) |
| `SIGUSR2` | Handler dyspozytora (zamknięcie dworca) | [dyspozytor.c:27](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L27) |
| `SIGUSR2` | Obsługa zamknięcia dworca | [dyspozytor.c:264-294](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L264-L294) |
| `SIGINT/SIGTERM` | Handler dyspozytora | [dyspozytor.c:29](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L29) |
| `SIGINT/SIGTERM` | Handler autobusu | [bus.c:15](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L15) |
| `SIGINT/SIGTERM` | Handler kasy | [kasa.c:13](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L13) |
| `SIGCHLD` | Handler zbierania zombie | [dyspozytor.c:33-36](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L33-L36) |
| `sigaction()` | Rejestracja handlerów dyspozytora | [dyspozytor.c:132-140](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L132-L140) |
| `sigaction()` | Rejestracja handlerów autobusu | [bus.c:37-40](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L37-L40) |
| `sigaction()` | Rejestracja handlerów kasy | [kasa.c:27-29](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L27-L29) |
| `kill()` | Wysyłanie SIGUSR1 do autobusu | [dyspozytor.c:259](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L259) |
| `kill()` | Wysyłanie SIGTERM do procesów | [dyspozytor.c:63-74](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L63-L74) |

### 5. Obsługa błędów i walidacja danych
| Element | Opis | Lokalizacja |
|---------|------|-------------|
| Walidacja parametrów | Sprawdzanie argc, zakresów N/P/R/T/K | [main.c:23-40](https://github.com/JaneckiGit/ProjektSO/blob/main/src/main.c#L23-L40) |
| Obsługa błędów IPC | Sprawdzanie zwracanych wartości semget/shmget/msgget | [dyspozytor.c:163-168](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L163-L168) |
| `perror()` | Błędy autobusu | [bus.c:48](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L48), [bus.c:105](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L105) |
| `perror()` | Błędy kasy | [kasa.c:39](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L39) |
| `perror()` | Błędy pasażera | [pasazer.c:288](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L288), [pasazer.c:301](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L301) |

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

---

## Użyte funkcje systemowe

### Procesy
| Funkcja | Lokalizacja |
|---------|-------------|
| `fork()` | [dyspozytor.c:199](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L199) (kasy), [dyspozytor.c:215](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L215) (autobusy), [dyspozytor.c:235](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L235) (generator), [pasazer.c:478](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L478) (pasażerowie) |
| `execl()` | [dyspozytor.c:206](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L206) (kasa), [dyspozytor.c:225](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L225) (autobus), [dyspozytor.c:241](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L241) (generator), [pasazer.c:485](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L485), [pasazer.c:487](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L487) (pasażerowie) |
| `waitpid()` | [dyspozytor.c:35](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L35), [dyspozytor.c:78](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L78), [dyspozytor.c:89](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L89), [dyspozytor.c:320](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L320) |
| `exit()` | [bus.c:49](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L49), [bus.c:342](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L342), [kasa.c:40](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L40), [kasa.c:130](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L130), [pasazer.c:132](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L132), [pasazer.c:186](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L186) |

### Wątki
| Funkcja | Lokalizacja |
|---------|-------------|
| `pthread_create()` | [pasazer.c:411](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L411) |
| `pthread_join()` | [pasazer.c:437](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L437), [pasazer.c:452](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L452) |
| `pthread_mutex_lock()` | [pasazer.c:21](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L21), [pasazer.c:432](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L432), [pasazer.c:447](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L447) |
| `pthread_mutex_unlock()` | [pasazer.c:25](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L25), [pasazer.c:435](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L435), [pasazer.c:450](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L450) |
| `pthread_cond_wait()` | [pasazer.c:24](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L24) |
| `pthread_cond_signal()` | [pasazer.c:434](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L434), [pasazer.c:449](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L449) |
| `pthread_mutex_destroy()` | [pasazer.c:438](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L438), [pasazer.c:453](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L453) |
| `pthread_cond_destroy()` | [pasazer.c:439](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L439), [pasazer.c:454](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L454) |
| `PTHREAD_MUTEX_INITIALIZER` | [pasazer.c:398](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L398) |
| `PTHREAD_COND_INITIALIZER` | [pasazer.c:399](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L399) |

### Sygnały
| Funkcja | Lokalizacja |
|---------|-------------|
| `sigaction()` | [dyspozytor.c:136-140](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L136-L140), [dyspozytor.c:147](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L147), [bus.c:37-40](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L37-L40), [kasa.c:27-29](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L27-L29) |
| `sigemptyset()` | [dyspozytor.c:131](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L131), [dyspozytor.c:146](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L146), [bus.c:36](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L36), [kasa.c:25](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L25) |
| `signal()` | [dyspozytor.c:120](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L120), [pasazer.c:459](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L459) |
| `kill()` | [dyspozytor.c:63](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L63), [dyspozytor.c:68](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L68), [dyspozytor.c:74](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L74), [dyspozytor.c:81-86](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L81-L86), [dyspozytor.c:259](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L259), [dyspozytor.c:273](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L273), [dyspozytor.c:278](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L278) |

### Semafory SysV
| Funkcja | Lokalizacja |
|---------|-------------|
| `ftok()` | [dyspozytor.c:150-153](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L150-L153), [utils.c:73-76](https://github.com/JaneckiGit/ProjektSO/blob/main/src/utils.c#L73-L76) |
| `semget()` | [dyspozytor.c:159](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L159), [utils.c:83](https://github.com/JaneckiGit/ProjektSO/blob/main/src/utils.c#L83) |
| `semctl()` | [dyspozytor.c:43-48](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L43-L48), [dyspozytor.c:52](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L52) |
| `semop()` | [bus.c:97](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L97), [bus.c:113](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L113), [bus.c:119-122](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L119-L122), [bus.c:284](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L284), [kasa.c:93](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L93), [kasa.c:104](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L104), [pasazer.c:92](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L92), [pasazer.c:127](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L127), [pasazer.c:199](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L199), [utils.c:28](https://github.com/JaneckiGit/ProjektSO/blob/main/src/utils.c#L28), [utils.c:61](https://github.com/JaneckiGit/ProjektSO/blob/main/src/utils.c#L61) |

### Pamięć dzielona SysV
| Funkcja | Lokalizacja |
|---------|-------------|
| `shmget()` | [dyspozytor.c:160](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L160), [utils.c:84](https://github.com/JaneckiGit/ProjektSO/blob/main/src/utils.c#L84) |
| `shmat()` | [dyspozytor.c:171](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L171), [dyspozytor.c:250](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L250), [dyspozytor.c:266](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L266), [dyspozytor.c:297](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L297), [bus.c:46](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L46), [kasa.c:37](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L37), [pasazer.c:300](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L300), [pasazer.c:366](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L366), [pasazer.c:463](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L463) |
| `shmdt()` | [dyspozytor.c:188](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L188), [dyspozytor.c:261](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L261), [dyspozytor.c:269](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L269), [bus.c:341](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L341), [kasa.c:129](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L129), [pasazer.c:132](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L132), [pasazer.c:186](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L186), [pasazer.c:466](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L466) |
| `shmctl()` | [dyspozytor.c:53](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L53) |

### Kolejki komunikatów SysV
| Funkcja | Lokalizacja |
|---------|-------------|
| `msgget()` | [dyspozytor.c:161-162](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L161-L162), [utils.c:85-86](https://github.com/JaneckiGit/ProjektSO/blob/main/src/utils.c#L85-L86) |
| `msgsnd()` | [bus.c:161](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L161), [bus.c:217](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L217), [bus.c:233](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L233), [bus.c:268](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L268), [bus.c:272](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L272), [kasa.c:61](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L61), [kasa.c:82](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L82), [kasa.c:106](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L106), [kasa.c:123](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L123), [pasazer.c:45](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L45), [pasazer.c:282](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L282) |
| `msgrcv()` | [bus.c:145](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L145), [bus.c:147](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L147), [bus.c:265](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L265), [bus.c:270](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L270), [kasa.c:53](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L53), [kasa.c:116](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L116), [pasazer.c:68](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L68), [pasazer.c:105](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L105), [pasazer.c:166](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L166), [pasazer.c:218](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L218), [pasazer.c:289](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L289) |
| `msgctl()` | [dyspozytor.c:54-55](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L54-L55), [dyspozytor.c:166-170](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L166-L170) |

### Pliki
| Funkcja | Lokalizacja |
|---------|-------------|
| `creat()` | [dyspozytor.c:190](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L190) |
| `open()` | [dyspozytor.c:93](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L93), [utils.c:54](https://github.com/JaneckiGit/ProjektSO/blob/main/src/utils.c#L54) |
| `write()` | [dyspozytor.c:111](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L111), [dyspozytor.c:116](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L116), [dyspozytor.c:193](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L193), [utils.c:46](https://github.com/JaneckiGit/ProjektSO/blob/main/src/utils.c#L46), [utils.c:56](https://github.com/JaneckiGit/ProjektSO/blob/main/src/utils.c#L56) |
| `close()` | [dyspozytor.c:117](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L117), [dyspozytor.c:194](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L194), [utils.c:57](https://github.com/JaneckiGit/ProjektSO/blob/main/src/utils.c#L57) |

---

## Testy 
```
TEST 1: Obciążeniowy - czy są odmowy przy małej pojemności?
TEST 2: Czy rowerzyści są odmawiani gdy R=2?
TEST 3: SIGUSR1 (N=2 P=10 R=3 T=15000 K=1)
TEST 4: SIGUSR2 (N=3 P=10 R=3 T=5000 K=2)
```
Uruchomienie: `./tests/test.sh [1|2|3|4|all]`

---

## Konfigurowalne parametry symulacji

### Parametry uruchomieniowe

| Parametr | Domyślna wartość | Opis | Lokalizacja |
|----------|------------------|------|-------------|
| `N` | 5 | Liczba autobusów | [main.c:9](https://github.com/JaneckiGit/ProjektSO/blob/main/src/main.c#L9) |
| `P` | 10 | Pojemność autobusu (miejsca normalne) | [main.c:10](https://github.com/JaneckiGit/ProjektSO/blob/main/src/main.c#L10) |
| `R` | 3 | Miejsca na rowery | [main.c:11](https://github.com/JaneckiGit/ProjektSO/blob/main/src/main.c#L11) |
| `T` | 5000 | Czas postoju na peronie [ms] | [main.c:12](https://github.com/JaneckiGit/ProjektSO/blob/main/src/main.c#L12) |
| `K` | 1 | Liczba kas biletowych | [main.c:13](https://github.com/JaneckiGit/ProjektSO/blob/main/src/main.c#L13) |

### Parametry autobusu

| Parametr | Wartość | Opis | Lokalizacja |
|----------|---------|------|-------------|
| `czas_trasy_Ti` | losuj(15000, 30000) | Czas trasy do miejsca docelowego [ms] | [bus.c:29](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L29) |
| `czas_dojazdu` (1. kurs) | losuj(1000, 2000) | Dojazd na dworzec [ms] | [bus.c:73](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L73) |
| `czas_dojazdu` (kolejne) | losuj(8000, 15000) | Powrót na dworzec [ms] | [bus.c:73](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L73) |

### Parametry pasażerów

| Parametr | Wartość | Opis | Lokalizacja |
|----------|---------|------|-------------|
| Wiek dorosłego | losuj(9, 80) | Zakres wieku pasażera | [pasazer.c:308](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L308) |
| Szansa na rower | 25% | Prawdopodobieństwo rowerzysty | [pasazer.c:309](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L309) |
| Szansa na VIP | 1% (tylko bez roweru) | Prawdopodobieństwo VIP | [pasazer.c:311](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L311) |
| Wiek dziecka | losuj(1, 7) | Zakres wieku dziecka (<8 lat) | [pasazer.c:375](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L375) |
| Wiek rodzica | losuj(18, 80) | Zakres wieku opiekuna | [pasazer.c:373](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L373) |

### Prawdopodobieństwa typów pasażerów

| Typ | Prawdopodobieństwo | Lokalizacja |
|-----|-------------------|-------------|
| Rodzic z dzieckiem | 20% | [pasazer.c:483](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L483) |
| Zwykły pasażer | 80% | [pasazer.c:485](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L485) |

---

## Specjalne zachowania

### VIP ([pasazer.c:150-189](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L150-L189))
- VIP **nie może mieć roweru** ([pasazer.c:310-311](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L310-L311))
- VIP **omija semafor drzwi** - wchodzi bez kolejki ([pasazer.c:150](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L150))
- VIP ma **priorytet w kolejce komunikatów** - mtype = bus_pid ([pasazer.c:33](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L33))
- VIP **omija kasę** - ma wcześniej wykupiony bilet ([pasazer.c:335-348](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L335-L348))

### Pasażer w trasie ([bus.c:196-199](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L196-L199))
- Licznik `pasazerow_w_trasie` jest inkrementowany **natychmiast przy wsiadaniu** (nie przy odjeździe)
- Zapewnia to poprawne śledzenie pasażerów nawet przy nagłym przerwaniu symulacji

### Atomowe zajęcie semaforów ([pasazer.c:83-92](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L83-L92))
- Pasażer z rowerem zajmuje **oba semafory atomowo** w jednym wywołaniu `semop()`
- Zapobiega to deadlockom między pasażerami a autobusem

### Retry przy wysyłaniu do kasy ([kasa.c:58-66](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L58-L66))
- Kasa próbuje wysłać odpowiedź do 5 razy przy pełnej kolejce
- Zapobiega to gubieniu odpowiedzi i wieszaniu pasażerów
