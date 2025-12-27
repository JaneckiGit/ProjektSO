#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <string.h>
#include "common.h"
#include "dyspozytor.h"

extern int shm_id;

static volatile sig_atomic_t zamknij_sygnal = 0;
static volatile int running = 1;

void handler_sigusr1(int sig) {
    (void)sig;
    SharedData *shm = (SharedData *)shmat(shm_id, NULL, 0);
    if (shm == (void *)-1) return;
    if (shm->aktualny_autobus_pid > 0) {
        log_print(KOLOR_DYSP, "DYSP", ">>> SYGNAL SIGUSR1: WYMUSZAM ODJAZD AUTOBUSU! <<<");
        kill(shm->aktualny_autobus_pid, SIGUSR1);
    } else {
        log_print(KOLOR_DYSP, "DYSP", "SIGUSR1: Brak autobusu na peronie.");
    }
    shmdt(shm);
}

void handler_sigusr2(int sig) {
    (void)sig;
    log_print(KOLOR_DYSP, "DYSP", ">>> SYGNAL SIGUSR2: ZAMYKAM DWORZEC! <<<");
    zamknij_sygnal = 1;
    running = 0;
    SharedData *shm = (SharedData *)shmat(shm_id, NULL, 0);
    if (shm != (void *)-1) {
        shm->stacja_otwarta = false;
        shm->wsiadanie_dozwolone = false;
        shmdt(shm);
    }
}

void proces_dyspozytor() {
    signal(SIGUSR1, handler_sigusr1);
    signal(SIGUSR2, handler_sigusr2);

    // Główna pętla dyspozytora
    while (running) {
        pause(); // oczekiwanie na sygnały
    }
}
