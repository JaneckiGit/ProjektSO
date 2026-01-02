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
// Sprawdza czy autobus powinien zakonczyc prace
static int czy_zakonczyc(SharedData *shm) {
    if (!shm->symulacja_aktywna) return 1;
    if (!shm->stacja_otwarta && shm->pasazerow_w_trasie <= 0) return 1;
    return 0;
}


//proces_autobus - Główna funkcja autobusu

void proces_autobus(int bus_id, int pojemnosc, int rowery, int czas_postoju) {
    srand(time(NULL) ^ getpid());
    
    // Losowy czas trasy Ti dla tego autobusu (15-30 sekund)
    int czas_trasy_Ti = losuj(15000, 30000);

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
    // struct sembuf zwolnij_peron = {SEM_BUS_STOP, 1, 0};
    // struct sembuf zablokuj_drzwi_n = {SEM_DOOR_NORMAL, -1, 0};
    // struct sembuf odblokuj_drzwi_n = {SEM_DOOR_NORMAL, 1, 0};
    // struct sembuf zablokuj_drzwi_r = {SEM_DOOR_ROWER, -1, 0};
    // struct sembuf odblokuj_drzwi_r = {SEM_DOOR_ROWER, 1, 0};
    struct sembuf zwolnij_peron = {SEM_BUS_STOP, 1, SEM_UNDO};
    struct sembuf zablokuj_drzwi_n = {SEM_DOOR_NORMAL, -1, SEM_UNDO};
    struct sembuf odblokuj_drzwi_n = {SEM_DOOR_NORMAL, 1, SEM_UNDO};
    struct sembuf zablokuj_drzwi_r = {SEM_DOOR_ROWER, -1, SEM_UNDO};
    struct sembuf odblokuj_drzwi_r = {SEM_DOOR_ROWER, 1, SEM_UNDO};

    int kursow = 0;
    int przewiezionych = 0;

    log_print(KOLOR_BUS, tag, "Miejsca normalne: %d, Miejsca rowerowe: %d, Ti=%dms. PID=%d", 
              pojemnosc, rowery, czas_trasy_Ti, getpid());

    while (bus_running) {
        if (czy_zakonczyc(shm)) {
            log_print(KOLOR_BUS, tag, "Warunki zakonczenia - koncze. PID=%d", getpid());
            break;
        }

        // JAZDA NA PETLE
        int czas_dojazdu = losuj(8000, 15000);
        log_print(KOLOR_BUS, tag, "Jedzie na petle (%dms). PID=%d", czas_dojazdu, getpid());
        
        int pozostalo = czas_dojazdu;
        while (pozostalo > 0 && bus_running) {
            int czekaj = (pozostalo > 500) ? 500 : pozostalo;
            msleep(czekaj);
            pozostalo -= czekaj;
            if (czy_zakonczyc(shm)) break;
        }

        if (!bus_running || czy_zakonczyc(shm)) {
            log_print(KOLOR_BUS, tag, "Warunki zakonczenia - koncze. PID=%d", getpid());
            break;
        }
        //ZAJĘCIE PERONU 
        log_print(KOLOR_BUS, tag, "Czeka na peron. PID=%d", getpid());

        // Non-blocking czekanie na peron (mozna przerwac)
        // struct sembuf zajmij_nowait = {SEM_BUS_STOP, -1, IPC_NOWAIT};
        struct sembuf zajmij_nowait = {SEM_BUS_STOP, -1, IPC_NOWAIT | SEM_UNDO};
        int proba_peronu = 0;
        while (proba_peronu < 100 && bus_running) {
            if (semop(sem_id, &zajmij_nowait, 1) == 0) break;
            if (errno != EAGAIN) { perror("bus semop zajmij_peron"); break; }
            if (czy_zakonczyc(shm)) {
                log_print(KOLOR_BUS, tag, "Warunki zakonczenia podczas czekania - koncze. PID=%d", getpid());
                goto koniec;
            }
            usleep(100000); // 100ms
            proba_peronu++;
        }
        
        if (proba_peronu >= 100) continue; // timeout, probuj ponownie

        if (czy_zakonczyc(shm)) {
            semop(sem_id, &zwolnij_peron, 1);
            log_print(KOLOR_BUS, tag, "Warunki zakonczenia po zajeciu peronu - koncze. PID=%d", getpid());
            break;
        }

        //AUTOBUS NA PERONIE 
        kursow++;
        wymuszony_odjazd = 0;

        // Aktualizacja stanu 
        struct sembuf shm_lock = {SEM_SHM, -1, SEM_UNDO};
        struct sembuf shm_unlock = {SEM_SHM, 1, SEM_UNDO};

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

        // SIGUSR2 - stacja zamknieta = nie przyjmuj pasazerow
        if (!shm->stacja_otwarta) {
            log_print(KOLOR_BUS, tag, "Stacja zamknieta - nie przyjmuje pasazerow, odjezdzam. PID=%d", getpid());
        } else {
            // POSTOJ - PRZYJMOWANIE PASAZEROW (tylko gdy stacja otwarta)
            while (czas_uplyniony < czas_postoju && !wymuszony_odjazd && bus_running && shm->stacja_otwarta) {
            // Odbieranie biletów 
            BiletMsg bilet;
            ssize_t ret = msgrcv(msg_id, &bilet, sizeof(BiletMsg) - sizeof(long), getpid(), IPC_NOWAIT);
            
            // Jesli brak VIP, sprawdz zwyklych pasazerow
            if (ret == -1 && errno == ENOMSG) {
                ret = msgrcv(msg_id, &bilet, sizeof(BiletMsg) - sizeof(long), getpid() + 1000000, IPC_NOWAIT);
            }

            if (ret != -1) {
                // Kierowca sprawdza bilet
                if (!bilet.ma_bilet) {
                    log_print(KOLOR_BUS, tag, "Kierowca: PAS %d BEZ BILETU - odmowa!", bilet.id_pasazera);
                    semop(sem_id, &shm_lock, 1);
                    shm->odrzuconych_bez_biletu++;
                    semop(sem_id, &shm_unlock, 1);
                    continue;
                }
                
                log_print(KOLOR_BUS, tag, "Kierowca: PAS %d - bilet OK.", bilet.id_pasazera);
                
                bool akceptuj = true;
                char powod[64] = "";

                semop(sem_id, &shm_lock, 1);

                int ile_osob = (bilet.wiek_dziecka > 0) ? 2 : 1;
                
                if (bilet.czy_rower) {
                    // Pasazer z rowerem - zajmuje TYLKO miejsce rowerowe
                    if (shm->rowery_zajete + ile_osob > rowery) {
                        akceptuj = false;
                        snprintf(powod, sizeof(powod), "brak miejsc rowerowych (%d/%d)",
                                 shm->rowery_zajete, rowery);
                    }
                } else {
                    // Pasazer bez roweru - zajmuje miejsce normalne
                    if (shm->miejsca_zajete + ile_osob > pojemnosc) {
                        akceptuj = false;
                        snprintf(powod, sizeof(powod), "brak miejsc normalnych (%d/%d)",
                                 shm->miejsca_zajete, pojemnosc);
                    }
                }

                if (akceptuj) {
                    if (bilet.czy_rower) {
                        shm->rowery_zajete += ile_osob;
                    } else {
                        shm->miejsca_zajete += ile_osob;
                    }
                    pasazerow_w_kursie += ile_osob;

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

                    // Autobus pelny (obie pule)
                    if (shm->miejsca_zajete >= pojemnosc && shm->rowery_zajete >= rowery) {
                        log_print(KOLOR_BUS, tag, "PELNY - odjazd! PID=%d", getpid());
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

       // TRASA (staly czas Ti dla tego autobusu)
        log_print(KOLOR_BUS, tag, "W trasie (%dms). PID=%d", czas_trasy_Ti, getpid());
        
        int pozostalo_trasa = czas_trasy_Ti;
        while (pozostalo_trasa > 0 && bus_running) {
            int czekaj = (pozostalo_trasa > 500) ? 500 : pozostalo_trasa;
            msleep(czekaj);
            pozostalo_trasa -= czekaj;
        }

        // Pasazerowie wysiadaja
        semop(sem_id, &shm_lock, 1);
        shm->pasazerow_w_trasie -= pasazerow_w_kursie;
        shm->total_przewiezionych += pasazerow_w_kursie;
        semop(sem_id, &shm_unlock, 1);

        log_print(KOLOR_BUS, tag, "Pasazerowie wysiedli (%d). PID=%d", pasazerow_w_kursie, getpid());

        // Jesli stacja zamknieta i pusty kurs - koncz
        if (pasazerow_w_kursie == 0 && !shm->stacja_otwarta) {
            log_print(KOLOR_BUS, tag, "Stacja zamknieta, pusty kurs - koncze. PID=%d", getpid());
            break;
        }
    }

    koniec:
    log_print(KOLOR_BUS, tag, "KONIEC. Kursow: %d, Przewiezionych: %d. PID=%d",
              kursow, przewiezionych, getpid());

    shmdt(shm);
    exit(0);
}
int main(int argc, char *argv[]) {
    if (argc < 5) {
        fprintf(stderr, "Uzycie: %s <bus_id> <pojemnosc> <rowery> <czas_postoju>\n", argv[0]);
        exit(1);
    }
    
    if (init_ipc_client() == -1) exit(1);
    
    proces_autobus(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));
    return 0;
}
