 //common.h - Plik nagłówkowy z definicjami współdzielonymi
 
#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>

// --- KONFIGURACJA SYMULACJI ---
#define MAX_PASAZEROW_TOTAL 100 // Całkowita liczba pasażerów do wygenerowania (zgodnie z wymaganiami)
#define LICZBA_AUTOBUSOW 5      // Łączna liczba autobusów [cite: 432]
#define POJEMNOSC_AUTOBUSU 10   // P [cite: 427]
#define MIEJSCA_ROWEROWE 3      // R [cite: 427]
#define CZAS_ODJAZDU 10         // T (sekundy) [cite: 429]
#define LIMIT_VIP 1             // Szansa na VIP w % (~1% jak w wymaganiach)

// (FTOK klucze nieużywane)

// --- SEMAFORY (INDEKSY) ---
#define SEM_MUTEX 0         // Ochrona pamięci dzielonej
#define SEM_PLATFORM 1      // Tylko jeden autobus na stanowisku
#define SEM_DOOR_PED 2      // Wejście dla pasażerów z bagażem podręcznym
#define SEM_DOOR_BIKE 3     // Wejście dla pasażerów z rowerami
#define SEM_KASA 4          // Kolejka do kasy

// --- TYPY KOMUNIKATÓW ---
#define MSG_LOG 1           // Typ komunikatu do logowania

// --- STRUKTURY DANYCH ---

// Pamięć dzielona - stan systemu
typedef struct {
    int aktualny_autobus_pid;
    int pasazerow_w_autobusie;       // Zajęte miejsca (z dziećmi liczy się 2 miejsca/osoba)
    int liczba_osob_w_autobusie;     // Rzeczywista liczba osób (każdy pasażer=1, nawet z dzieckiem)
    int rowerow_w_autobusie;
    bool stacja_otwarta;
    bool autobus_na_stanowisku;
    int licznik_biletow;
    int total_passengers_generated;  // Całkowita liczba wygenerowanych
    int passengers_waiting;          // Czekający na dworcu
    int passengers_in_transit;       // W autobusach
} SharedState;

// Struktura komunikatu do logowania (raport)
typedef struct {
    long mtype;
    char text[256];
} LogMessage;

// Kolory do terminala dla czytelności
#define COLOR_RESET "\033[0m"
#define COLOR_BUS "\033[1;33m"      // Żółty
#define COLOR_PASS "\033[0;36m"     // Cyjan
#define COLOR_KASA "\033[0;32m"     // Zielony
#define COLOR_DISP "\033[1;31m"     // Czerwony
#define COLOR_INFO "\033[0;35m"     // Fioletowy (komunikaty statusu)

// --- PROTOTYPY FUNKCJI NARZĘDZIOWYCH ---
void send_log(int msgid, const char *format, ...);
void sem_p(int semid, int sem_num);
void sem_v(int semid, int sem_num);
int random_range(int min, int max);
void get_timestamp(char *buffer);

#endif