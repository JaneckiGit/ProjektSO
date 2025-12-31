// common.h - Wspólne definicje i struktury
#ifndef COMMON_H
#define COMMON_H

#include <sys/types.h>
#include <stdio.h> 
#include <stdlib.h>
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

// Stałe
#define MAX_BUSES           50
#define MAX_CAPACITY        200
#define MAX_REGISTERED      1000
#define MAX_CZEKAJACE_DZIECI 100
#define MAX_KASY            10
#define DEFAULT_K           1

// Kolory ANSI
#define KOLOR_RESET   "\033[0m"
#define KOLOR_MAIN    "\033[1;37m"
#define KOLOR_KASA    "\033[1;32m"
#define KOLOR_BUS     "\033[1;33m"
#define KOLOR_DYSP    "\033[1;31m"
#define KOLOR_PAS     "\033[1;36m"
#define KOLOR_STAT    "\033[1;35m"

// Indeksy semaforów
#define SEM_DOOR_NORMAL  0
#define SEM_DOOR_ROWER   1
#define SEM_BUS_STOP     2
#define SEM_LOG          3
#define SEM_SHM          4
#define SEM_KASA_BASE    5
#define SEM_COUNT_BASE   5

// Struktura dziecka czekającego na rodzica
typedef struct {
    pid_t pid_dziecka;
    int id_dziecka;
    int wiek_dziecka;
    bool ma_rodzica;
    pid_t pid_rodzica;
} CzekajaceDziecko;

// Pamięć dzielona
typedef struct {
    int param_N;
    int param_P;
    int param_R;
    int param_T;
    int param_K;
    bool stacja_otwarta;
    bool symulacja_aktywna;
    
    pid_t aktualny_bus_pid;
    int aktualny_bus_id;
    int miejsca_zajete;
    int rowery_zajete;
    bool bus_na_peronie;
    
    int total_pasazerow;
    int pasazerow_w_trasie;
    int pasazerow_czeka;
    int sprzedanych_biletow;
    int vip_count;
    int total_przewiezionych;
    int odrzuconych_bez_biletu;
    int obsluzonych_kasa[MAX_KASY];

    pid_t registered_pids[MAX_REGISTERED];
    int registered_wiek[MAX_REGISTERED];
    int registered_count;
    
    // Dzieci czekające na rodziców
    CzekajaceDziecko dzieci[MAX_CZEKAJACE_DZIECI];
    int dzieci_count;
} SharedData;

// Wiadomość - bilet
typedef struct {
    long mtype;
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

// Zmienne IPC
extern int sem_id;
extern int shm_id;
extern int msg_id;

// Funkcje pomocnicze
void log_print(const char* kolor, const char* tag, const char* fmt, ...);
int losuj(int min, int max);
void msleep(int ms);
void get_timestamp(char* buf, size_t size);
int init_ipc_client(void);  // dla procesow potomnych

#endif
