#ifndef COMMON_H
#define COMMON_H

#include <sys/types.h>
#include <sys/ipc.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>

// Klucze IPC
#define KEY_SEM  0x1234
#define KEY_SHM  0x5678
#define KEY_MSG  0x9ABC

// Definicje kolorów (ANSI)
#define KOLOR_RESET   "\033[0m"
#define KOLOR_DYSP    "\033[1;31m" // Czerwony
#define KOLOR_KASA    "\033[1;32m" // Zielony
#define KOLOR_BUS     "\033[1;33m" // Żółty
#define KOLOR_PAS     "\033[1;36m" // Cyjan

// Indeksy semaforów
#define SEM_DOOR_NORMAL  0 // Wejście dla pasażerów z bagażem (mutex)
#define SEM_DOOR_ROWER   1 // Wejście dla pasażerów z rowerem (mutex)
#define SEM_BUS_STOP     2 // Miejsce na przystanku (mutex)
#define SEM_KASA_1       3 // Okienko kasowe 1 (mutex)
#define SEM_KASA_2       4 // Okienko kasowe 2 (mutex)
#define SEM_LOG          5 // Synchronizacja logowania
#define LICZBA_SEM       6

// Struktura Pamięci Dzielonej (wymagane nazwy polskie)
typedef struct {
    int aktualny_autobus_pid;        // PID autobusu stojącego na peronie (0 = brak)
    int pasazerow_w_autobusie;       // Licznik miejsc zajętych (z uwzględnieniem dzieci)
    int liczba_osob_w_autobusie;     // Rzeczywista liczba głów (do logowania)
    int rowerow_w_autobusie;         // Licznik rowerów
    bool stacja_otwarta;             // Flaga działania stacji (czy generator tworzy pasażerów)
    bool wsiadanie_dozwolone;        // Flaga czy pasażerowie mogą wsiadać (SIGUSR2 = false)
    bool autobus_na_stanowisku;      // Czy autobus fizycznie stoi
    int licznik_biletow;             // Globalny licznik sprzedanych biletów
    int total_passengers_generated;  // Statystyka całkowita
    int passengers_waiting;          // Czekający na dworcu (statystyka)
    int passengers_in_transit;       // W autobusach (statystyka)
    int vip_waiting;                 // VIP-y czekające (priorytet)
} SharedData;

// Struktura wiadomości (Bilet)
typedef struct {
    long mtype;       // PID autobusu (adresat)
    pid_t pid_pasazera;
    int czy_rower;    // 1 - tak, 0 - nie
    int czy_vip;      // 1 - tak, 0 - nie
    int ile_miejsc;   // 1 lub 2 (jeśli z dzieckiem)
    int wiek;         // Wiek pasażera (dziecko <8 z rodzicem)
    int z_dzieckiem;  // 1 - rodzic z dzieckiem <8 lat
} BiletMsg;

// Funkcje pomocnicze (utils)
void log_print(const char* kolor, const char* tag, const char* fmt, ...);
int losuj(int min, int max);
void m_sleep(int ms);
long aktualny_czas_ms();

#endif
