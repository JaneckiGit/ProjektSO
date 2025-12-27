#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include "common.h"
#include "bus.h"

extern int sem_id, shm_id, msg_id;
static volatile int force_depart = 0;
static volatile int running = 1;

void handler_bus_sigusr1(int sig) {
    (void)sig;
    force_depart = 1;
}

void handler_bus_sigusr2(int sig) {
    (void)sig;
    running = 0;
}

void ustaw_sygnaly_busa() {
    signal(SIGUSR1, handler_bus_sigusr1);
    signal(SIGUSR2, handler_bus_sigusr2);
}

void proces_autobus(int id_wew, int max_p, int max_r, int czas_postoju) {
    ustaw_sygnaly_busa();


    char bus_tag[16];
    snprintf(bus_tag, sizeof(bus_tag), "BUS %d", id_wew);

    log_print(KOLOR_BUS, bus_tag, "Uruchomiony. PID=%d, Pojemnosc=%d, Rowery=%d", 
              getpid(), max_p, max_r);

    SharedData *shm = (SharedData *)shmat(shm_id, NULL, 0);
    if (shm == (void *)-1) {
        perror("bus shmat");
        exit(1);
    }
    
    struct sembuf parkuj = {SEM_BUS_STOP, -1, 0};
    struct sembuf odjazd_sem = {SEM_BUS_STOP, 1, 0};
    struct sembuf lock_door_normal = {SEM_DOOR_NORMAL, -1, 0};
    struct sembuf unlock_door_normal = {SEM_DOOR_NORMAL, 1, 0};
    struct sembuf lock_door_rower = {SEM_DOOR_ROWER, -1, 0};
    struct sembuf unlock_door_rower = {SEM_DOOR_ROWER, 1, 0};

    int pasazerow_przewiezionych = 0;

    while (running) {
        if (!shm->stacja_otwarta && shm->passengers_in_transit <= 0) {
            log_print(KOLOR_BUS, bus_tag, "Stacja zamknieta i brak pasazerow. Koncze prace.");
            break;
        }
        
        int trip_time = losuj(2000, 5000);
        log_print(KOLOR_BUS, bus_tag, "Jedzie na petle (czas: %dms)...", trip_time);
        m_sleep(trip_time);
        
        log_print(KOLOR_BUS, bus_tag, "Czeka na wjazd na peron.");
        if (semop(sem_id, &parkuj, 1) == -1) {
            perror("bus semop parkuj");
            break;
        }

        shm->aktualny_autobus_pid = getpid();
        shm->autobus_na_stanowisku = true;
        shm->pasazerow_w_autobusie = 0;
        shm->liczba_osob_w_autobusie = 0;
        shm->rowerow_w_autobusie = 0;

        log_print(KOLOR_BUS, bus_tag, "Podstawil sie na peron. Wolne miejsca: %d, Miejsca na rowery: %d", 
                  max_p, max_r);

        force_depart = 0;
        int elapsed = 0;
        int pasazerow_w_tym_kursie = 0;
        
        while (elapsed < czas_postoju && !force_depart && running && shm->stacja_otwarta) {
            BiletMsg msg;
            
            if (msgrcv(msg_id, &msg, sizeof(BiletMsg) - sizeof(long), getpid(), IPC_NOWAIT) != -1) {
                bool can_enter = true;
                char reject_reason[64] = "";
                
                if (shm->pasazerow_w_autobusie + msg.ile_miejsc > max_p) {
                    can_enter = false;
                    snprintf(reject_reason, sizeof(reject_reason), "brak miejsc (%d/%d)", 
                             shm->pasazerow_w_autobusie, max_p);
                }
                if (msg.czy_rower && shm->rowerow_w_autobusie >= max_r) {
                    can_enter = false;
                    snprintf(reject_reason, sizeof(reject_reason), "brak miejsc na rowery (%d/%d)", 
                             shm->rowerow_w_autobusie, max_r);
                }

                if (can_enter) {
                    shm->pasazerow_w_autobusie += msg.ile_miejsc;
                    shm->liczba_osob_w_autobusie++;
                    if (msg.czy_rower) shm->rowerow_w_autobusie++;
                    pasazerow_w_tym_kursie += msg.ile_miejsc;
                    
                    log_print(KOLOR_BUS, bus_tag, "Przyjeto pasazera PID=%d (wiek=%d%s%s). Stan: %d/%d, Rowery: %d/%d",
                        msg.pid_pasazera, msg.wiek,
                        msg.z_dzieckiem ? " +dziecko" : "",
                        msg.czy_vip ? " VIP" : "",
                        shm->pasazerow_w_autobusie, max_p, shm->rowerow_w_autobusie, max_r);
                } else {
                    log_print(KOLOR_DYSP, bus_tag, "ODMOWA dla PID=%d: %s", msg.pid_pasazera, reject_reason);
                }
            } else {
                if (errno != ENOMSG) perror("msgrcv");
            }
            usleep(100000);
            elapsed += 100;
        }

        if (force_depart) {
            log_print(KOLOR_DYSP, bus_tag, ">>> WYMUSZONY ODJAZD (SIGUSR1) <<<");
        }
        
        log_print(KOLOR_BUS, bus_tag, "Zamykanie drzwi (oba wejscia)...");
        
        if (semop(sem_id, &lock_door_normal, 1) == -1) perror("bus lock normal");
        if (semop(sem_id, &lock_door_rower, 1) == -1) perror("bus lock rower");
        
        shm->aktualny_autobus_pid = 0;
        shm->autobus_na_stanowisku = false;
        
        int zabrani_w_kursie = pasazerow_w_tym_kursie;
        shm->passengers_in_transit += zabrani_w_kursie;
        pasazerow_przewiezionych += zabrani_w_kursie;

        log_print(KOLOR_BUS, bus_tag, "ODJAZD! Zabral %d osob (lacznie przewiozl: %d). W trasie: %d", 
                  shm->liczba_osob_w_autobusie, pasazerow_przewiezionych, shm->passengers_in_transit);
        
        if (semop(sem_id, &unlock_door_normal, 1) == -1) perror("bus unlock normal");
        if (semop(sem_id, &unlock_door_rower, 1) == -1) perror("bus unlock rower");
        if (semop(sem_id, &odjazd_sem, 1) == -1) perror("bus odjazd_sem");

        int route_time = losuj(3000, 6000);
        log_print(KOLOR_BUS, bus_tag, "W trasie (czas: %dms)...", route_time);
        m_sleep(route_time);
        
        shm->passengers_in_transit -= zabrani_w_kursie;
        log_print(KOLOR_BUS, bus_tag, "Pasazerowie wysiedli (%d). W trasie pozostalo: %d", 
                  zabrani_w_kursie, shm->passengers_in_transit);
    }

    log_print(KOLOR_BUS, bus_tag, "Koniec pracy. Lacznie przewiozl %d pasazerow.", pasazerow_przewiezionych);
    shmdt(shm);
    exit(0);
}
