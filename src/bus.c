// bus.c - Modul autobusu
#include "bus.h"
#include "common.h"

// Flagi sygnałów 
static volatile sig_atomic_t wymuszony_odjazd = 0;
static volatile sig_atomic_t bus_running = 1;


//Handler sygnałów autobusu
 
static void handler_bus(int sig) {
    if (sig == SIGUSR1) {
        wymuszony_odjazd = 1;
    } else if (sig == SIGINT || sig == SIGTERM) {
        bus_running = 0;
    }
}


//proces_autobus - Główna funkcja autobusu

void proces_autobus(int bus_id, int pojemnosc, int rowery, int czas_postoju) {
    srand(time(NULL) ^ getpid());

    //Konfiguracja sygnałów 
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handler_bus;
    sa.sa_flags = SA_RESTART;
    sigemptyset(&sa.sa_mask);

    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    char tag[16];
    snprintf(tag, sizeof(tag), "BUS %d", bus_id);

    // Podłączenie do pamięci 
    SharedData *shm = (SharedData *)shmat(shm_id, NULL, 0);
    if (shm == (void *)-1) {
        perror("bus shmat");
        exit(1);
    }

    // Operacje semaforowe 
    struct sembuf zajmij_peron = {SEM_BUS_STOP, -1, 0};
    struct sembuf zwolnij_peron = {SEM_BUS_STOP, 1, 0};
    struct sembuf zablokuj_drzwi_n = {SEM_DOOR_NORMAL, -1, 0};
    struct sembuf odblokuj_drzwi_n = {SEM_DOOR_NORMAL, 1, 0};
    struct sembuf zablokuj_drzwi_r = {SEM_DOOR_ROWER, -1, 0};
    struct sembuf odblokuj_drzwi_r = {SEM_DOOR_ROWER, 1, 0};

    int kursow = 0;
    int przewiezionych = 0;

    //
    while (bus_running && shm->symulacja_aktywna) {
        //Sprawdź czy jest sens kontynuować 
        if (!shm->stacja_otwarta && shm->pasazerow_w_trasie <= 0 && 
            shm->pasazerow_czeka <= 0) {
            break;
        }

        //JAZDA NA PĘTLĘ 
        int czas_dojazdu = losuj(2000, 4000);
        log_print(KOLOR_BUS, tag, "Jedzie na pętlę (%dms). PID=%d", czas_dojazdu, getpid());
        msleep(czas_dojazdu);

        if (!bus_running || !shm->symulacja_aktywna) break;

        //ZAJĘCIE PERONU 
        log_print(KOLOR_BUS, tag, "Czeka na wjazd na peron. PID=%d", getpid());

        if (semop(sem_id, &zajmij_peron, 1) == -1) {
            if (errno == EINTR) continue;
            break;
        }

        //AUTOBUS NA PERONIE 
        kursow++;
        wymuszony_odjazd = 0;

        // Aktualizacja stanu 
        struct sembuf shm_lock = {SEM_SHM, -1, 0};
        struct sembuf shm_unlock = {SEM_SHM, 1, 0};

        semop(sem_id, &shm_lock, 1);
        shm->aktualny_bus_pid = getpid();
        shm->aktualny_bus_id = bus_id;
        shm->bus_na_peronie = true;
        shm->miejsca_zajete = 0;
        shm->rowery_zajete = 0;
        semop(sem_id, &shm_unlock, 1);

        log_print(KOLOR_BUS, tag, "[Kurs #%d] Na peronie. PID=%d, Miejsca: 0/%d, Rowery: 0/%d",
                  kursow, getpid(), pojemnosc, rowery);

        // POSTÓJ - PRZYJMOWANIE PASAŻERÓW 
        int czas_uplyniony = 0;
        int pasazerow_w_kursie = 0;

        while (czas_uplyniony < czas_postoju && !wymuszony_odjazd && bus_running) {
            // Odbieranie biletów 
            BiletMsg bilet;
            ssize_t ret = msgrcv(msg_id, &bilet, sizeof(BiletMsg) - sizeof(long),
                                 getpid(), IPC_NOWAIT);

            if (ret != -1) {
                // Sprawdzenie limitów 
                bool akceptuj = true;
                char powod[64] = "";

                semop(sem_id, &shm_lock, 1);

                if (shm->miejsca_zajete + bilet.ile_miejsc > pojemnosc) {
                    akceptuj = false;
                    snprintf(powod, sizeof(powod), "brak miejsc (%d/%d)",
                             shm->miejsca_zajete, pojemnosc);
                }
                if (bilet.czy_rower && shm->rowery_zajete >= rowery) {
                    akceptuj = false;
                    snprintf(powod, sizeof(powod), "brak miejsc na rowery (%d/%d)",
                             shm->rowery_zajete, rowery);
                }

                if (akceptuj) {
                    shm->miejsca_zajete += bilet.ile_miejsc;
                    if (bilet.czy_rower) shm->rowery_zajete++;
                    pasazerow_w_kursie += bilet.ile_miejsc;

                    semop(sem_id, &shm_unlock, 1);

                    if (bilet.wiek_dziecka > 0) {
                        log_print(KOLOR_BUS, tag,
                                  "Wsiadł PAS %d (PID=%d, wiek=%d) z dzieckiem (wiek=%d)%s. "
                                  "Miejsca: %d/%d, Rowery: %d/%d",
                                  bilet.id_pasazera, bilet.pid_pasazera, bilet.wiek,
                                  bilet.wiek_dziecka, bilet.czy_vip ? " VIP" : "",
                                  shm->miejsca_zajete, pojemnosc,
                                  shm->rowery_zajete, rowery);
                    } else {
                        log_print(KOLOR_BUS, tag,
                                  "Wsiadł PAS %d (PID=%d, wiek=%d)%s%s. "
                                  "Miejsca: %d/%d, Rowery: %d/%d",
                                  bilet.id_pasazera, bilet.pid_pasazera, bilet.wiek,
                                  bilet.czy_rower ? " +rower" : "",
                                  bilet.czy_vip ? " VIP" : "",
                                  shm->miejsca_zajete, pojemnosc,
                                  shm->rowery_zajete, rowery);
                    }

                    // Autobus pełny 
                    if (shm->miejsca_zajete >= pojemnosc) {
                        log_print(KOLOR_BUS, tag, "PEŁNY! Odjazd. PID=%d", getpid());
                        break;
                    }
                } else {
                    semop(sem_id, &shm_unlock, 1);
                    log_print(KOLOR_BUS, tag, "Odmowa PAS %d (PID=%d): %s",
                              bilet.id_pasazera, bilet.pid_pasazera, powod);
                }
            }

            usleep(100000);  // 100ms 
            czas_uplyniony += 100;
        }

        // ODJAZD wymuszony
        if (wymuszony_odjazd) {
            log_print(KOLOR_BUS, tag, ">>> WYMUSZONY ODJAZD (SIGUSR1)! PID=%d <<<", getpid());
        }

        // Zablokuj drzwi na czas odjazdu 
        semop(sem_id, &zablokuj_drzwi_n, 1);
        semop(sem_id, &zablokuj_drzwi_r, 1);

        // Aktualizacja stanu busa
        semop(sem_id, &shm_lock, 1);
        shm->aktualny_bus_pid = 0;
        shm->aktualny_bus_id = 0;
        shm->bus_na_peronie = false;
        shm->pasazerow_w_trasie += pasazerow_w_kursie;
        przewiezionych += pasazerow_w_kursie;
        int w_trasie = shm->pasazerow_w_trasie;
        semop(sem_id, &shm_unlock, 1);

        log_print(KOLOR_BUS, tag, "ODJAZD! Zabrał %d osób. W trasie: %d. PID=%d",
                  pasazerow_w_kursie, w_trasie, getpid());

        //Odblokuj drzwi i peron 
        semop(sem_id, &odblokuj_drzwi_n, 1);
        semop(sem_id, &odblokuj_drzwi_r, 1);
        semop(sem_id, &zwolnij_peron, 1);

        //TRASA 
        int czas_trasy = losuj(3000, 6000);
        log_print(KOLOR_BUS, tag, "W trasie (%dms). PID=%d", czas_trasy, getpid());
        msleep(czas_trasy);

        // Pasażerowie wysiadają 
        semop(sem_id, &shm_lock, 1);
        shm->pasazerow_w_trasie -= pasazerow_w_kursie;
        int pozostalo = shm->pasazerow_w_trasie;
        semop(sem_id, &shm_unlock, 1);

        log_print(KOLOR_BUS, tag, "Pasażerowie wysiedli (%d). W trasie: %d. PID=%d",
                  pasazerow_w_kursie, pozostalo, getpid());
    }

    log_print(KOLOR_BUS, tag, "KONIEC. Kursów: %d, Przewiezionych: %d. PID=%d",
              kursow, przewiezionych, getpid());

    shmdt(shm);
    exit(0);
}
