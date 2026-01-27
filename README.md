# Projekt Systemy Operacyjne - Autobus Podmiejski

**Autor:** Mateusz Janecki  
**Numer albumu:** 155182  
**Kierunek:** Informatyka  
**Rok:** 2  
**Semestr:** 3  
**Grupa Projektowa:** 1  
**Przedmiot:** Systemy Operacyjne (Projekt)

**Repozytorium GitHub:** https://github.com/JaneckiGit/ProjektSO/

---

## 1. Opis projektu

Na dworcu stoi autobus o pojemności P pasażerów, w którym jednocześnie można przewieźć R rowerów. Do autobusu są dwa wejścia (każde może pomieścić jedną osobę): jedno dla pasażerów z bagażem podręcznym i drugie dla pasażerów z rowerami. Autobus odjeżdża co określoną ilość czasu T. W momencie odjazdu kierowca musi dopilnować, aby na wejściach nie było żadnego wchodzącego pasażera. Jednocześnie musi dopilnować, by liczba wszystkich pasażerów autobusu nie przekroczyła P i liczba pasażerów z rowerami nie była większa niż R.

Po odjeździe autobusu na jego miejsce pojawia się natychmiast (jeżeli jest dostępny) nowy pojazd o takiej samej pojemności jak poprzedni. Łączna liczba autobusów wynosi N, każdy o pojemności P z R miejscami na rowery.

Pasażerowie w różnym wieku przychodzą na dworzec w losowych momentach czasu. Przed wejściem na przystanek kupują bilet w kasie (K). Istnieje pewna liczba osób VIP (ok. 1%) – posiadająca wcześniej wykupiony bilet, które nie płacą za bilet i mogą wejść na przystanek i do autobusu z pominięciem kolejki oczekujących.

Dzieci w wieku poniżej 8 roku życia mogą wejść do autobusu tylko pod opieką osoby dorosłej (dziecko zajmuje osobne miejsce w autobusie). Kasa rejestruje wszystkie wchodzące osoby (ID procesu/wątku). Każdy pasażer przy wejściu do autobusu okazuje kierowcy ważny bilet – do autobusu nie może wejść osoba bez wykupionego wcześniej biletu.

Autobusy przewożą pasażerów do miejsca docelowego i po czasie Ti (wartość losowa, każdy autobus ma inny czas) wracają na dworzec. Na polecenie dyspozytora (sygnał 1) autobus, który w danym momencie stoi na dworcu może odjechać z niepełną liczbą pasażerów. Po otrzymaniu od dyspozytora polecenia (sygnał 2) pasażerowie nie mogą wsiąść do żadnego autobusu - nie mogą wejść na dworzec. Autobusy kończą pracę po rozwiezieniu wszystkich pasażerów.

---

## 2. Założenia projektowe

### 2.1 Kompilacja i uruchomienie

```bash
make clean     # czyszczenie plików
make           # kompilacja wszystkiego
make run       # uruchomienie (domyślne parametry)
make clean-ipc # czyszczenie zasobów IPC
```

Uruchomienie z parametrami:
```bash
./bin/autobus_main [N] [P] [R] [T] [K]
```
### 2.2 Środowisko testowe

**System operacyjny:** Debian GNU/Linux 11 (bullseye)

**Kernel:** Linux 6.12.54-linuxkit

**Kompilator:** gcc 10.2.1

**IDE:** Visual Studio Code z rozszerzeniem Dev Containers

**Konteneryzacja:** Docker z konfiguracją w `.devcontainer/`

### 2.3 Tryb testowy

Projekt posiada przełącznik `TRYB_TESTOWY` w pliku [common.h](https://github.com/JaneckiGit/ProjektSO/blob/main/include/common.h) (linia 6).

W trybie testowym: T=10ms, Ti=0ms, 10000 pasażerów, brak opóźnień. W trybie produkcyjnym: T=5000ms, Ti=15-30s, nieograniczona liczba pasażerów, odstępy 1-2s.

**Miejsca w kodzie gdzie TRYB_TESTOWY wpływa na działanie:**
- [main.c linia 12](https://github.com/JaneckiGit/ProjektSO/blob/main/src/main.c#L12) - domyślna wartość T
- [bus.c linia 35](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L35) - czas trasy Ti
- [bus.c linia 75](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L75) - czas dojazdu na dworzec
- [pasazer.c linia 679](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L679) - liczba pasażerów i odstępy

---

### 2.4 Struktura projektu

```
ProjektSO/
├── .devcontainer/                 # Konfiguracja Docker
│   ├── devcontainer.json
│   ├── Dockerfile
│   └── reinstall-cmake.sh
├── include/                       # Pliki nagłówkowe
│   ├── common.h                   # Definicje wspólne
│   ├── bus.h, dyspozytor.h, kasa.h, pasazer.h
├── src/                           # Pliki źródłowe
│   ├── main.c                     # Punkt wejścia
│   ├── dyspozytor.c               # Zarządzanie symulacją
│   ├── bus.c                      # Logika autobusu
│   ├── kasa.c                     # Obsługa kasy
│   ├── pasazer.c                  # Pasażerowie i generator
│   └── utils.c                    # Funkcje pomocnicze
├── bin/                           # Pliki wykonywalne
├── tests/                         # Skrypty testowe
├── Makefile
└── raport.txt                     # Raport z symulacji
```

---
### 2.5 Założenia zadania

- Na dworzec przybywają pasażerowie w losowych odstępach czasowych
- Pasażer może mieć rower (25% szans)
- VIP (1% szans, tylko bez roweru) omija kolejkę do kasy i ma priorytet w autobusie
- Rodzic z dzieckiem (20% szans) - jeden proces, dwa wątki
- Dzieci poniżej 8 lat mogą wejść tylko pod opieką dorosłego
- Na autobus może wejść maksymalnie P pasażerów
- Miejsca na rowery ograniczone do R
- Autobus ma dwa wejścia - normalne i rowerowe (semafory binarne)
- Kierowca sprawdza bilet każdego pasażera
- Autobus odjeżdża co T czasu lub po sygnale SIGUSR1
- SIGUSR2 zamyka dworzec - nowi pasażerowie nie wchodzą

### 2.6 Architektura systemu

System oparty jest na architekturze wieloprocesowej z następującymi procesami:

| Proces | Opis |
|--------|------|
| **Main/Dyspozytor** | Proces główny - tworzy zasoby IPC, uruchamia pozostałe procesy, obsługuje sygnały |
| **Autobus** | Zarządza pojazdem, przyjmuje pasażerów, kontroluje bilety, realizuje kursy |
| **Kasa** | Sprzedaje bilety, rejestruje pasażerów |
| **Pasażer** | Próbuje kupić bilet i wsiąść do autobusu |
| **Generator** | Tworzy nowych pasażerów w losowych odstępach czasu |

### 2.7 Parametry systemu

| Parametr | Domyślna wartość | Opis |
|----------|------------------|------|
| N | 1 | Liczba autobusów |
| P | 100 | Pojemność autobusu (miejsca normalne) |
| R | 20 | Liczba miejsc na rowery |
| T | 10 (test) / 5000 (prod) | Czas postoju na peronie [ms] |
| K | 1 | Liczba kas biletowych |

### 2.8 Walidacja danych

Wszystkie wprowadzane parametry są walidowane:
- N musi być w zakresie 1-50 (MAX_BUSES)
- P musi być w zakresie 1-200000 (MAX_CAPACITY)
- R musi być w zakresie 0-P
- T musi być większe od 0
- K musi być w zakresie 1-10 (MAX_KASY)

---

## 3. Opis implementacji

### 3.1 Mechanizmy IPC

#### 3.1.1 Pamięć dzielona

Pamięć dzielona służy do przechowywania wspólnych stanów systemu - struktura `SharedData` w [common.h](https://github.com/JaneckiGit/ProjektSO/blob/main/include/common.h):

```c
typedef struct {
    // Parametry symulacji
    int param_N, param_P, param_R, param_T, param_K;
    
    // Flagi stanu
    bool dworzec_otwarty;      // false = SIGUSR2 zamknął dworzec
    bool symulacja_aktywna;    // false = koniec symulacji
    
    // Stan autobusu na peronie
    pid_t aktualny_bus_pid;
    int aktualny_bus_id;
    int miejsca_zajete;
    int rowery_zajete;
    bool bus_na_przystanku;
    
    // Statystyki i rejestracja pasażerów
    // ...
} SharedData;
```

#### 3.1.2 Semafory

System wykorzystuje 7 semaforów do synchronizacji:

| Semafor | Indeks | Init | Funkcja |
|---------|--------|------|---------|
| SEM_DOOR_NORMAL | 0 | 1 | Drzwi normalne - kontrola wejścia do autobusu |
| SEM_DOOR_ROWER | 1 | 1 | Drzwi rowerowe - kontrola wejścia z rowerem |
| SEM_BUS_STOP | 2 | 1 | Peron - tylko jeden autobus naraz |
| SEM_LOG | 3 | 1 | Synchronizacja dostępu do logów |
| SEM_SHM | 4 | 1 | Ochrona pamięci dzielonej |
| SEM_KASA_STRAZNIK | 5 | 200 | Ograniczenie kolejki do kas |
| SEM_BUS_SIGNAL | 6 | 1 | Sygnalizacja przyjazdu/odjazdu autobusu |

#### 3.1.3 Kolejki komunikatów

System wykorzystuje dwie kolejki komunikatów:

**msg_id** - komunikacja pasażer ↔ autobus (BiletMsg, OdpowiedzMsg)

**msg_kasa_id** - komunikacja pasażer ↔ kasa (KasaRequest, KasaResponse)

#### 3.1.4 Sygnały

| Sygnał | Nadawca | Odbiorca | Działanie |
|--------|---------|----------|-----------|
| SIGUSR1 | Dyspozytor | Autobus | Wymuszony odjazd przed czasem T |
| SIGUSR2 | Dyspozytor | Procesy | Zamknięcie dworca |
| SIGTERM | Dyspozytor | Wszystkie | Zakończenie symulacji |
| SIGINT | System | Dyspozytor | Natychmiastowe przerwanie |
| SIGCHLD | Kernel | Dyspozytor | Automatyczne zbieranie zombie |
| SIGALRM | System | Pasażer/Kasa | Timeout operacji blokujących |

---
