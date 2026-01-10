// kasa.c - Moduł kasy biletowej (jako osobny proces)
// Każda kasa = osobny proces
// Komunikacja przez kolejkę komunikatów msg_kasa_id

#include "common.h"
#include "kasa.h"

// Flaga sygnałów
static volatile sig_atomic_t kasa_running = 1;

// Handler sygnałów kasy
static void handler_kasa(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        kasa_running = 0;
    }
}

// Główna funkcja kasy (jeden proces = jedna kasa)
void proces_kasa(int numer_kasy) {
    srand(time(NULL) ^ getpid());

    // Konfiguracja sygnałów
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handler_kasa;
    sa.sa_flags = SA_RESTART;
    sigemptyset(&sa.sa_mask);

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    char tag[16];
    snprintf(tag, sizeof(tag), "KASA %d", numer_kasy);

    // Podłączenie do pamięci dzielonej
    SharedData *shm = (SharedData *)shmat(shm_id, NULL, 0);
    if (shm == (void *)-1) {
        perror("kasa shmat");
        exit(1);
    }

    struct sembuf shm_lock = {SEM_SHM, -1, SEM_UNDO};
    struct sembuf shm_unlock = {SEM_SHM, 1, SEM_UNDO};

    int obsluzonych = 0;

    log_print(KOLOR_KASA, tag, "Otwarta. PID=%d", getpid());

    // Główna pętla - odbiera żądania z kolejki
    while (kasa_running && shm->symulacja_aktywna) {
        KasaRequest req;
        
        // IPC_NOWAIT - nie blokuj jeśli brak żądań
        ssize_t ret = msgrcv(msg_kasa_id, &req, sizeof(KasaRequest) - sizeof(long), 
                             numer_kasy, IPC_NOWAIT);
        
        if (ret != -1) {
            log_print(KOLOR_KASA, tag, "Obsluguje PAS %d (wiek=%d, biletow=%d)", 
                      req.id_pasazera, req.wiek, req.ile_biletow);
            
            // Symulacja czasu obsługi
            msleep(losuj(200, 500));
            
            // Aktualizacja statystyk
            semop(sem_id, &shm_lock, 1);
            if (shm->registered_count < MAX_REGISTERED) {
                shm->registered_pids[shm->registered_count] = req.pid_pasazera;
                shm->registered_wiek[shm->registered_count] = req.wiek;
                shm->registered_count++;
            }
            shm->sprzedanych_biletow += req.ile_biletow;
            shm->obsluzonych_kasa[numer_kasy - 1]++;
            semop(sem_id, &shm_unlock, 1);
            
            // Odpowiedź do pasażera
            KasaResponse resp;
            resp.mtype = req.pid_pasazera;
            resp.numer_kasy = numer_kasy;
            resp.sukces = 1;
            msgsnd(msg_kasa_id, &resp, sizeof(KasaResponse) - sizeof(long), 0);
            
            log_print(KOLOR_KASA, tag, "Sprzedano %d bilet(y) PAS %d", 
                      req.ile_biletow, req.id_pasazera);
            
            obsluzonych++;
        } else {
            // Brak żądań - krótkie oczekiwanie
            usleep(50000);  // 50ms
        }
    }

    log_print(KOLOR_KASA, tag, "Zamknieta. Obsluzono: %d. PID=%d", obsluzonych, getpid());
    
    shmdt(shm);
    exit(0);
}

int main(int argc, char *argv[]) {
    if (argc < 2) { 
        fprintf(stderr, "Uzycie: %s <numer_kasy>\n", argv[0]); 
        exit(1); 
    }
    
    if (init_ipc_client() == -1) exit(1);
    
    int numer_kasy = atoi(argv[1]);
    if (numer_kasy <= 0 || numer_kasy > MAX_KASY) { 
        fprintf(stderr, "Blad: numer_kasy musi byc 1-%d\n", MAX_KASY); 
        exit(1); 
    }
    
    proces_kasa(numer_kasy);
    return 0;
}