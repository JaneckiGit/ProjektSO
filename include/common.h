// common.h - Wspólne definicje i struktury
#ifndef COMMON_H
#define COMMON_H

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <fcntl.h>
#include <pthread.h>

// Stałe Stałe konfiguracyjne (MAX_BUSES, MAX_CAPACITY, etc.)
#define MAX_BUSES           50 // Maksymalna liczba autobusów
#define MAX_CAPACITY        200 // Maksymalna pojemność autobusu
#define MAX_REGISTERED      1000 // Maksymalna liczba zarejestrowanych pasażerów
#define MAX_CZEKAJACE_DZIECI 100 // Maksymalna liczba dzieci czekających na opiekunów
#define MAX_KASY            10 // Maksymalna liczba kas biletowych

// Kolory ANSI do logow
#define KOLOR_RESET   "\033[0m" // Reset
#define KOLOR_MAIN    "\033[1;37m" // Biały
#define KOLOR_KASA    "\033[1;32m" // Zielony
#define KOLOR_BUS     "\033[1;33m" // Żółty
#define KOLOR_DYSP    "\033[1;31m" // Czerwony
#define KOLOR_PAS     "\033[1;36m" // Cyan
#define KOLOR_STAT    "\033[1;35m"  // Magenta

// Indeksy semaforów
#define SEM_DOOR_NORMAL  0 // drzwi do autobusu
#define SEM_DOOR_ROWER   1 // drzwi do autobusu z rowerem
#define SEM_BUS_STOP     2 // peron - tylko jeden autobus
#define SEM_LOG          3 // Log
#define SEM_SHM          4 // pamięć dzielona
#define SEM_COUNT        5 // liczba semaforów przed kasami

// Struktura dziecka czekającego na rodzica
//Czekajace Dziecko - dziecko <8 lat czekające PRZED dworcem na opiekuna
typedef struct {
    pid_t pid_dziecka; 
    int id_dziecka;
    int wiek_dziecka;
    bool ma_rodzica;
    pid_t pid_rodzica;
} CzekajaceDziecko;

// Pamięć dzielona miedzy wszystkie procesy
//Dostęp chroniony przez SEM_SHM
typedef struct {
    int param_N;
    int param_P;
    int param_R;
    int param_T;
    int param_K;
    // Flagi stanu
    bool stacja_otwarta; // false = dworzec zamknięty (SIGUSR2)
    bool symulacja_aktywna; // false = koniec symulacji
    // Stan autobusu na peronie
    pid_t aktualny_bus_pid;
    int aktualny_bus_id;
    int miejsca_zajete;
    int rowery_zajete;
    bool bus_na_peronie;
    // Statystyki
    int total_pasazerow;
    int pasazerow_w_trasie;
    int pasazerow_czeka;
    int sprzedanych_biletow;
    int vip_count;
    int total_przewiezionych;
    int odrzuconych_bez_biletu;
    int obsluzonych_kasa[MAX_KASY];
    // Rejestracja pasażerów (do sprawdzania biletów)
    pid_t registered_pids[MAX_REGISTERED];
    int registered_wiek[MAX_REGISTERED];
    int registered_count;
    
    // Dzieci czekające na rodziców
    CzekajaceDziecko dzieci[MAX_CZEKAJACE_DZIECI];
    int dzieci_count;
} SharedData;

// Wiadomość - bilet
typedef struct {
    long mtype; //okresla priorytet 
    pid_t pid_pasazera;
    int id_pasazera;
    int wiek;
    int czy_rower;
    int czy_vip;
    int ma_bilet;
    // Dane dziecka (jeśli jest)
    pid_t pid_dziecka;
    int id_dziecka;
    int wiek_dziecka;
} BiletMsg;

// Wiadomość - żądanie do kasy (pasażer -> kasa)
typedef struct {
    long mtype;              // = numer_kasy (1, 2, K)
    pid_t pid_pasazera;      // PID pasażera (do odpowiedzi)
    int id_pasazera;
    int wiek;
    int ile_biletow;         // 1 lub 2 (z dzieckiem)
} KasaRequest;

// Wiadomość - odpowiedź z kasy (kasa -> pasażer)
typedef struct {
    long mtype;              // = PID pasażera
    int numer_kasy;          // Która kasa obsłużyła
    int sukces;              // 1 = bilet kupiony
} KasaResponse;

// Zmienne IPC
extern int sem_id; // id semaforów
extern int shm_id; // id pamięci dzielonej
extern int msg_id; // id kolejki wiadomości dla autobusów
extern int msg_kasa_id; // id kolejki wiadomości dla kas

// Funkcje pomocnicze
void log_print(const char* kolor, const char* tag, const char* fmt, ...);// Logi z kolorami i tagami
int losuj(int min, int max);// Losowa liczba z zakresu [min, max]
void msleep(int ms);// Uspienie na ms milisekund
void get_timestamp(char* buf, size_t size);// Pobranie aktualnego timestampu
int init_ipc_client(void);  // dla procesow potomnych

#endif
