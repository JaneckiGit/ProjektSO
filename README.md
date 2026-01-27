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
- [bus.c linia 37](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L37) - czas trasy Ti
- [bus.c linia 80](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L80) - czas dojazdu na dworzec
- [pasazer.c linia 710](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L710) - liczba pasażerów i odstępy

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

### 2.7 Parametry systemu (main.c)

| Parametr | Domyślna wartość | Opis |
|----------|------------------|------|
| N | 1 | Liczba autobusów |
| P | 100 | Pojemność autobusu (miejsca normalne) |
| R | 20 | Liczba miejsc na rowery |
| T | 10 (test) / 5000 (prod) | Czas postoju na peronie [ms] |
| K | 1 | Liczba kas biletowych |

### 2.8 Walidacja danych (main.c)

Wszystkie wprowadzane parametry są walidowane:
- N musi być w zakresie 1-(MAX_BUSES)
- P musi być w zakresie 1-(MAX_CAPACITY)
- R musi być w zakresie 0-P
- T musi być większe od 0
- K musi być w zakresie 1-10 (MAX_KASY)

---

## 3. Opis implementacji

### 3.1 Mechanizmy IPC

#### 3.1.1 Pamięć dzielona

Pamięć dzielona służy do przechowywania wspólnych stanów systemu - struktura `SharedData` w [common.h](https://github.com/JaneckiGit/ProjektSO/blob/main/include/common.h):

```
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

## 4. Opis procesów

### 4.1 Proces główny - [dyspozytor.c](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L142)

Główna funkcja `proces_dyspozytor()` (linia 142) odpowiada za:
- Konfigurację handlerów sygnałów
- Tworzenie zasobów IPC (ftok, semget, shmget, msgget)
- Inicjalizację semaforów i pamięci dzielonej
- Uruchomienie procesów kas, autobusów i generatora pasażerów
- Główną pętlę obsługi sygnałów SIGUSR1/SIGUSR2
- Graceful shutdown i cleanup zasobów IPC

### 4.2 Proces autobusu - [bus.c](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L32)

Główna funkcja `proces_autobus()` (linia 32) realizuje:
- Losowanie indywidualnego czasu trasy Ti
- Konfigurację handlerów sygnałów
- Główną pętlę kursów (jazda na dworzec, postój, przyjmowanie pasażerów, odjazd)
- Weryfikację biletów i kontrolę miejsc
- Obsługę wymuszonego odjazdu (SIGUSR1)

### 4.3 Proces kasy - [kasa.c](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L22)

Główna funkcja `proces_kasa()` (linia 22) realizuje:
- Konfigurację handlerów sygnałów
- Główną pętlę odbierania żądań od pasażerów (blokujące msgrcv)
- 2% szans na odmowę sprzedaży (brak środków pasażera)
- Rejestrację pasażerów w pamięci dzielonej
- Wysyłanie odpowiedzi z retry i timeout

### 4.4 Procesy pasażerów - [pasazer.c](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c)

**Typy pasażerów:**

1. **Normal** - funkcja [`proces_pasazer()`](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L512) (linia 512): wiek 8-80 lat, 25% szans na rower, 1% szans na VIP (tylko bez roweru), kupuje 1 bilet

2. **Rodzic z dzieckiem** - funkcja [`proces_rodzic_z_dzieckiem()`](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L585) (linia 585): rodzic 18-80 lat, dziecko 1-7 lat jako wątek, kupuje 2 bilety, synchronizacja przez mutex i zmienną warunkową

3. **Generator** - funkcja [`proces_generator()`](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L703) (linia 703): w trybie testowym 10000 pasażerów bez opóźnień, w trybie produkcyjnym nieograniczona liczba z odstępami 1-2s, 20% rodzic z dzieckiem, 80% zwykły

### 4.5 Specjalne zachowania

**VIP** - nie może mieć roweru, omija semafor drzwi (wchodzi bez kolejki), ma priorytet w kolejce komunikatów (mtype = bus_pid), omija kasę (ma wcześniej wykupiony bilet)

**Rowerzysta** - zajmuje semafory sekwencyjnie: najpierw DOOR_NORMAL, potem DOOR_ROWER, sprawdza czy autobus nie odjechał między zajęciem semaforów

---

## 5. Sekcje krytyczne kodu

### 5.1 Dostęp do pamięci dzielonej

Każdy dostęp do pamięci dzielonej jest chroniony semaforem SEM_SHM z flagą SEM_UNDO i obsługą EINTR.

**Kod:** [pasazer.c linie 72-73](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L72-L73) - definicja operacji semaforowych, [pasazer.c linie 229-231](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L229-L231) - przykład użycia z obsługą EINTR

### 5.2 Sekwencyjne zajęcie drzwi przez rowerzystę

Pasażer z rowerem zajmuje semafory **sekwencyjnie** - najpierw DOOR_NORMAL, potem DOOR_ROWER. Użycie `semtimedop` z timeoutem pozwala na okresowe sprawdzanie czy autobus nie odjechał. Po każdym zajęciu semafora sprawdzamy stan autobusu. Zapobiega to zakleszczeniu gdy autobus odjeżdża podczas oczekiwania.

**Kod:** [pasazer.c linie 119-122](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L119-L122) - definicja operacji na semaforach drzwi, [pasazer.c linie 127-150](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L127-L150) - sekwencyjne zajmowanie z timeout

### 5.3 Zamykanie drzwi przed odjazdem

Autobus przed zamknięciem drzwi ustawia `bus_na_przystanku=false`. Pasażerowie sprawdzają tę flagę i zwalniają semafory. Autobus używa `semtimedop` z timeoutem i wieloma próbami.

**Kod:** [bus.c linie 287-289](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L287-L289) - ustawienie flagi przed odjazdem, [bus.c linie 310-324](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L310-L324) - zamykanie drzwi z timeout i wieloma próbami

### 5.4 Synchronizacja rodzic-dziecko (wątki)

Zmienna warunkowa (`pthread_cond_t`) zapewnia efektywne oczekiwanie wątku dziecka bez busy-wait. Mutex chroni dostęp do flagi `zakoncz`.

**Kod:** [pasazer.c linie 641-643](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L641-L643) - inicjalizacja mutex, cond i flagi, [pasazer.c linie 23-27](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L23-L27) - oczekiwanie wątku dziecka, [pasazer.c linie 673-676](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L673-L676) - sygnalizacja zakończenia

### 5.5 Obsługa sygnałów

Handlery sygnałów używają `volatile sig_atomic_t` dla bezpiecznej komunikacji. Handler tylko ustawia flagę - obsługa w głównej pętli.

**Kod:** [bus.c linie 12-13](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L12-L13) - deklaracja flag volatile, [bus.c linie 16-21](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L16-L21) - handler ustawiający flagi, [bus.c linia 72](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L72) - sprawdzanie flagi w głównej pętli


---

## 6. Środowisko testowe

**System operacyjny:** Debian GNU/Linux 11 (bullseye)

**Kernel:** Linux 6.12.54-linuxkit

**Kompilator:** gcc 10.2.1

**IDE:** Visual Studio Code z rozszerzeniem Dev Containers

**Konteneryzacja:** Docker z konfiguracją w `.devcontainer/`

### 6.1 Tryb testowy

Projekt posiada przełącznik `TRYB_TESTOWY` w pliku [common.h](https://github.com/JaneckiGit/ProjektSO/blob/main/include/common.h) (linia 6).

W trybie testowym: T=10ms, Ti=0ms, 10000 pasażerów, brak opóźnień. W trybie produkcyjnym: T=5000ms, Ti=15-30s, nieograniczona liczba pasażerów, odstępy 1-2s.



---

## 7. Struktura projektu

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

## 8. Testy

### 8.1 TEST 1: zabicie procesu pasażera w drzwiach ZALICZONY
**Cel:Weryfikacja działania SEM_UNDO - automatycznego zwalniania semaforów gdy proces trzymający semafor zostanie zabity. Test sprawdza czy system nie blokuje się na zawsze gdy pasażer "umrze" trzymając semafor drzwi.
**Opis:Zatrzymanie procesu pasażera gdy trzyma semafor drzwi. Inni pasażerowie i autobus muszą czekać. Po zabiciu procesu (SIGKILL), SEM_UNDO powinno automatycznie ZWOLNIĆ semafor. modyfikacja w pliku pasazer dodanie dodatkowego loga oraz dodanie przedłużonego czasu
**Wynik: Po SIGKILL semafor zostaje automatycznie zwolniony (SEM_UNDO)
Symulacja kontynuuje normalnie

<img width="578" height="178" alt="Zrzut ekranu 2026-01-27 o 23 02 08" src="https://github.com/user-attachments/assets/14226419-bd0b-4f60-86cd-133befa2f0a9" />

<img width="620" height="53" alt="Zrzut ekranu 2026-01-27 o 23 02 23" src="https://github.com/user-attachments/assets/e0bb841c-59f2-4813-acff-5780d3270124" />

<img width="601" height="41" alt="Zrzut ekranu 2026-01-27 o 23 02 31" src="https://github.com/user-attachments/assets/9093bbfe-4919-4c4c-be39-77e30d95df21" />

<img width="613" height="236" alt="Zrzut ekranu 2026-01-27 o 23 02 45" src="https://github.com/user-attachments/assets/13990746-b6e0-4ce6-97dc-311cb1b88d91" />

<img width="607" height="75" alt="Zrzut ekranu 2026-01-27 o 23 03 11" src="https://github.com/user-attachments/assets/ac695000-7066-4446-be16-e7c3bb74d1cd" />

### 8.2 TEST 2: Obsługa sygnału SIGUSR1 (Wymuszony odjazd) ZALICZONY
**Cel: Potwierdzenie, że dyspozytor potrafi wymusić natychmiastowy odjazd autobusu stojącego na peronie przed upływem czasu postoju.
**Opis: Autobus nie może czekać pełnych 15 sekund. W logach musi pojawić się komunikat potwierdzający odbiór sygnału: „>>> WYMUSZONY ODJAZD (SIGUSR1)”.

<img width="605" height="118" alt="Zrzut ekranu 2026-01-27 o 23 04 05" src="https://github.com/user-attachments/assets/8fb0cb1a-637f-4ea8-bed0-4e3c1b82bb75" />

### 8.3 TEST 3: Obciążeniowy – Jedna kasa ZALICZONY
**Cel: Sprawdzenie stabilności kolejek IPC oraz spójności danych statystycznych przy ekstremalnym obciążeniu jednego punktu sprzedaży.
**Opis: Uruchomienie symulacji z parametrami K=1 oraz 10 000 pasażerów. Test weryfikuje wydajność kolejki komunikatów msg_kasa_id oraz poprawność działania semafora SEM_KASA_STRAZNIK, który ogranicza liczbę osób mogących jednocześnie oczekiwać na obsługę. Kluczowe jest sprawdzenie, czy przy tak dużej liczbie operacji na pamięci dzielonej (SEM_SHM) nie dochodzi do utraty danych.

<img width="398" height="116" alt="Zrzut ekranu 2026-01-27 o 20 16 29" src="https://github.com/user-attachments/assets/b1a1698d-fce8-44a0-aa76-c7c6cdbd6753" />

<img width="381" height="209" alt="Zrzut ekranu 2026-01-27 o 20 16 45" src="https://github.com/user-attachments/assets/865098a1-3fc4-4c32-95f7-0c932374e971" />

<img width="594" height="367" alt="Zrzut ekranu 2026-01-27 o 20 29 31" src="https://github.com/user-attachments/assets/e7ea29d8-1707-48df-a283-2eb66adcd782" />

### 8.4 TEST 4: Blokada sprzedaży biletów (Brak środków) ZALICZONY
**Cel: Weryfikacja zachowania systemu w sytuacji całkowitego braku sprzedaży biletów w kasach oraz obsługa pasażerów odrzuconych przez kierowcę.
**Opis: Modyfikacja prawdopodobieństwa w module kasa.c na 100% szans na odmowę (brak środków). Uruchomienie symulacji dla 5000 pasażerów przy dużej liczbie autobusów. Test sprawdza, czy pasażerowie bez biletów są poprawnie identyfikowani przez kierowcę, odrzucani i czy statystyka opuscilo_bez_jazdy oraz odrzuconych_bez_biletu w pamięci dzielonej (SharedData) jest poprawnie aktualizowana.

<img width="601" height="223" alt="Zrzut ekranu 2026-01-27 o 23 09 21" src="https://github.com/user-attachments/assets/e4749ccf-dc52-4a6a-80ac-3ae4da2fbe44" />

<img width="607" height="187" alt="Zrzut ekranu 2026-01-27 o 23 09 31" src="https://github.com/user-attachments/assets/3f0b8539-1a51-4d8d-a5a0-623bbd401a25" />

<img width="608" height="279" alt="Zrzut ekranu 2026-01-27 o 23 09 37" src="https://github.com/user-attachments/assets/18c74b18-6c2e-400e-8d4f-0525308599e7" />

```bash
./bin/autobus_main 3 10 3 5000 2 &
pgrep -f autobus_main
kill -SIGUSR1 <PID>  # wymuszony odjazd
kill -SIGUSR2 <PID>  # zamknięcie dworca
```

---

## 9. Zrealizowane funkcjonalności

- Wieloprocesowa architektura z fork() i exec()
- Synchronizacja semaforami System V
- Pamięć dzielona do wymiany danych
- Kolejki komunikatów (2 kanały)
- Obsługa sygnałów SIGUSR1, SIGUSR2, SIGTERM, SIGINT
- Wątki POSIX (rodzic z dzieckiem)
- Mutex i zmienne warunkowe
- VIP z priorytetem i omijaniem kolejki
- Kontrola biletów przez kierowcę
- Dwa typy drzwi (normalne i rowerowe)
- System logowania z kolorami i plikiem
- Walidacja parametrów wejściowych
- Raport końcowy ze statystykami
- Graceful shutdown (SIGUSR2)
- Automatyczne zbieranie zombie (SIGCHLD)
- Czyszczenie zasobów IPC przy zakończeniu
- Tryb testowy dla szybkich testów

---

## 10. Napotkane problemy

### 10.1 Zakleszczenie przy zamykaniu drzwi
**Problem:** Autobus blokował się przy próbie zamknięcia drzwi gdy pasażer był w trakcie wchodzenia.
**Rozwiązanie:** Przed zamknięciem drzwi autobus ustawia `bus_na_przystanku=false`. Pasażerowie sprawdzają tę flagę i zwalniają semafory. Autobus używa `semtimedop` z timeoutem.

### 10.2 Duplikaty pasażerów
**Problem:** Ten sam pasażer mógł wysłać bilet do dwóch różnych autobusów.
**Rozwiązanie:** Pasażer trzyma semafor drzwi przez cały czas od wysłania biletu do otrzymania odpowiedzi. Dodatkowo pamięta PID ostatniego autobusu który go odrzucił.

### 10.3 Zombie processes
**Problem:** Duża liczba procesów pasażerów powodowała gromadzenie się zombie.
**Rozwiązanie:** `signal(SIGCHLD, SIG_IGN)` dla automatycznego zbierania, handler SIGCHLD z `waitpid(WNOHANG)` w dyspozytorze, wątek czyszczący w tle.

### 10.5 Pełna kolejka komunikatów
**Problem:** Przy dużej liczbie pasażerów kolejka komunikatów zapełniała się.
**Rozwiązanie:** utworzenie semafora STRAŻNIKA .

---

## 11. Linki do fragmentów kodu

### 11.1 Tworzenie i obsługa procesów

| Funkcja | Plik | Linia |
|---------|------|-------|
| fork() - tworzenie kas | dyspozytor.c | [235](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L235) |
| fork() - tworzenie autobusów | dyspozytor.c | [252](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L252) |
| fork() - tworzenie generatora | dyspozytor.c | [273](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L273) |
| fork() - tworzenie pasażerów | pasazer.c | [728](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L728) |
| execl() - uruchomienie kasy | dyspozytor.c | [245](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L245) |
| execl() - uruchomienie autobusu | dyspozytor.c | [265](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L265) |
| execl() - uruchomienie generatora | dyspozytor.c | [280](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L280) |
| execl() - uruchomienie pasażera | pasazer.c | [738](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L738) |
| waitpid() - zbieranie zombie | dyspozytor.c | [35](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L35) |
| exit() - zakończenie autobusu | bus.c | [409](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L409) |
| exit() - zakończenie kasy | kasa.c | [180](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L80) |

### 11.2 Obsługa wątków (pthread)

| Funkcja | Plik | Linia |
|---------|------|-------|
| pthread_create() - wątek dziecka | pasazer.c | [649](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L649) |
| pthread_join() - oczekiwanie na wątek | pasazer.c | [674](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L674) |
| pthread_mutex_lock() | pasazer.c | [25](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L25) |
| pthread_mutex_unlock() | pasazer.c | [30](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L30) |
| pthread_cond_wait() | pasazer.c | [28](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L28) |
| pthread_cond_signal() | pasazer.c | [672](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L672) |
| watek_dziecko() - funkcja wątku | pasazer.c | [17](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L17) |

### 11.3 Obsługa sygnałów

| Funkcja | Plik | Linia |
|---------|------|-------|
| handler_dyspozytor() | dyspozytor.c | [23](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L23) |
| handler_bus() | bus.c | [16](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L16) |
| handler_kasa() | kasa.c | [13](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L13) |
| handler_sigchld() | dyspozytor.c | [33](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L33) |
| sigaction() - rejestracja dyspozytora | dyspozytor.c | [154](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L154) |
| sigaction() - rejestracja autobusu | bus.c | [48](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L48) |
| sigaction() - rejestracja kasy | kasa.c | [31](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L31) |
| kill() - wysyłanie SIGUSR1 | dyspozytor.c | [295](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L295) |
| kill() - wysyłanie SIGTERM | dyspozytor.c | [78](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L78) |

### 11.4 Semafory System V

| Funkcja | Plik | Linia |
|---------|------|-------|
| ftok() - generowanie kluczy | dyspozytor.c | [169](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L169) |
| semget() - tworzenie semaforów | dyspozytor.c | [178](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L178) |
| semctl() - inicjalizacja semaforów | dyspozytor.c | [43](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L43) |
| semctl(IPC_RMID) - usuwanie | dyspozytor.c | [68](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L68) |
| semop() - blokada drzwi (rowerzysta) | pasazer.c | [144](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L144) |
| semop() - blokada drzwi (normalny) | pasazer.c | [334](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L334) |

### 11.5 Pamięć dzielona System V

| Funkcja | Plik | Linia |
|---------|------|-------|
| shmget() - tworzenie segmentu | dyspozytor.c | [179](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L179) |
| shmat() - przyłączenie (dyspozytor) | dyspozytor.c | [203](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L203) |
| shmat() - przyłączenie (autobus) | bus.c | [55](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L55) |
| shmat() - przyłączenie (kasa) | kasa.c | [43](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L43) |
| shmat() - przyłączenie (pasażer) | pasazer.c | [521](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L521) |
| shmdt() - odłączenie | dyspozytor.c | [220](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L220) |
| shmctl(IPC_RMID) - usuwanie | dyspozytor.c | [69](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L69) |

### 11.6 Kolejki komunikatów System V

| Funkcja | Plik | Linia |
|---------|------|-------|
| msgget() - tworzenie kolejek | dyspozytor.c | [180](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L180) |
| msgsnd() - wysyłanie biletu | pasazer.c | [52](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L52) |
| msgrcv() - odbieranie biletów | bus.c | [180](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L180) |
| msgsnd() - odpowiedź autobusu | bus.c | [252](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L252) |
| msgsnd() - żądanie do kasy | pasazer.c | [460](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L460) |
| msgrcv() - żądanie od pasażera (kasa) | kasa.c | [61](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L61) |
| msgsnd() - odpowiedź kasy | kasa.c | [140](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L140) |
| msgrcv() - odpowiedź z kasy | pasazer.c | [486](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L486) |
| msgctl(IPC_RMID) - usuwanie | dyspozytor.c | [70](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L70) |

### 11.7 Główne funkcje procesów

| Funkcja | Plik | Linia |
|---------|------|-------|
| proces_dyspozytor() | dyspozytor.c | [142](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L142) |
| proces_autobus() | bus.c | [29](https://github.com/JaneckiGit/ProjektSO/blob/main/src/bus.c#L29) |
| proces_kasa() | kasa.c | [20](https://github.com/JaneckiGit/ProjektSO/blob/main/src/kasa.c#L20) |
| proces_pasazer() | pasazer.c | [509](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L509) |
| proces_rodzic_z_dzieckiem() | pasazer.c | [581](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L581) |
| proces_generator() | pasazer.c | [702](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L702) |

### 11.8 Funkcje pomocnicze

| Funkcja | Plik | Linia |
|---------|------|-------|
| init_semafory() | dyspozytor.c | [40](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L40) |
| cleanup_ipc() | dyspozytor.c | [67](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L67) |
| shutdown_children() | dyspozytor.c | [75](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L75) |
| zapisz_raport_koncowy() | dyspozytor.c | [111](https://github.com/JaneckiGit/ProjektSO/blob/main/src/dyspozytor.c#L111) |
| czekaj_na_autobus() | pasazer.c | [67](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L67) |
| kup_bilet() | pasazer.c | [409](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L409) |
| wyslij_bilet() | pasazer.c | [34](https://github.com/JaneckiGit/ProjektSO/blob/main/src/pasazer.c#L34) |
| log_print() | utils.c | [23](https://github.com/JaneckiGit/ProjektSO/blob/main/src/utils.c#L23) |
| init_ipc_client() | utils.c | [75](https://github.com/JaneckiGit/ProjektSO/blob/main/src/utils.c#L75) |

---

## 12. Podsumowanie

Projekt "Autobus Podmiejski" został zrealizowany zgodnie ze specyfikacją oraz wymaganiami projektowymi. Zaimplementowano wszystkie wymagane funkcjonalności:

1. **Wieloprocesowa symulacja** z użyciem `fork()` i `exec()`
2. **Synchronizacja** za pomocą semaforów System V
3. **Dwa mechanizmy komunikacji IPC:** pamięć dzielona (stan systemu) i kolejki komunikatów (pasażer ↔ autobus, pasażer ↔ kasa)
4. **Obsługa sygnałów** SIGUSR1, SIGUSR2, SIGTERM, SIGINT, SIGCHLD
5. **Wątki POSIX** dla rodzica z dzieckiem 
6. **Prawidłowe czyszczenie zasobów** IPC przy zakończeniu
