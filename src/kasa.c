//Moduł kasy biletowej
//Każda kasa=osobny proces
//Komunikacja przez kolejkę komunikatow msg_kasa_id
#include "common.h"
#include "kasa.h"

//Flaga sygnalow kasy
static volatile sig_atomic_t kasa_running = 1;

//Handler sygnalow kasy
static void handler_kasa(int sig) {
    if (sig == SIGINT || sig == SIGTERM || sig == SIGQUIT) {
        kasa_running = 0;
    }
}
//Glowna funkcja kasy biletowej
void proces_kasa(int numer_kasy) {
    srand(time(NULL) ^ getpid());

    //Konfiguracja sygnalow
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handler_kasa;
    sa.sa_flags = SA_RESTART;
    sigemptyset(&sa.sa_mask);
    //sigaction() obsluga sygnalow SIGINT/SIGTERM
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGQUIT, &sa, NULL);

    char tag[16];//tag do logow
    snprintf(tag, sizeof(tag), "KASA %d", numer_kasy);

    //podlaczenie do pamieci dzielonej
    SharedData *shm = (SharedData *)shmat(shm_id, NULL, 0);
    if (shm == (void *)-1) {
        perror("kasa shmat"); 
        exit(1);
    }
    //operacje na semaforach
    struct sembuf shm_lock = {SEM_SHM, -1, SEM_UNDO};
    struct sembuf shm_unlock = {SEM_SHM, 1, SEM_UNDO};

    int obsluzonych = 0;//liczba obsluzonych pasazerow
    //time_t obsluga_start, obsluga_koniec;  //do mierzenia czasu obslugi
    log_print(KOLOR_KASA, tag, "Otwarta. PID=%d", getpid());

    //glowna petla  odbiera wiadomosci od pasazerow
    while (kasa_running && shm->symulacja_aktywna) {
        KasaRequest req;
        //IPC_NOWAIT nie bedzie blokowane jesli nie mawiadomosci
        ssize_t ret = msgrcv(msg_kasa_id, &req, sizeof(KasaRequest) - sizeof(long), numer_kasy, IPC_NOWAIT);
        if (ret != -1) {
            //Jesli dworzec zamkniety nie obsluguj
            if (!shm->dworzec_otwarty) {
                KasaResponse resp;
                resp.mtype = req.pid_pasazera;
                resp.numer_kasy = numer_kasy;
                resp.sukces = 0;
                resp.brak_srodkow = 0;
                int retry = 0;
                while (msgsnd(msg_kasa_id, &resp, sizeof(KasaResponse) - sizeof(long), IPC_NOWAIT) == -1 && retry < 5) {
                    if (errno == EAGAIN) { 
                        //usleep(5000); 
                        retry++; continue; }
                    break;
                }
                continue;
            }
            log_print(KOLOR_KASA, tag, "Obsluguje PAS %d (wiek=%d, biletow=%d)", req.id_pasazera, req.wiek, req.ile_biletow);
            // //symulacja czasu obslugi 
            // obsluga_start = time(NULL);
            // obsluga_koniec = obsluga_start + 1;  
            // while (time(NULL) < obsluga_koniec && kasa_running && shm->dworzec_otwarty) {
            // }
            KasaResponse resp;
            resp.mtype = req.pid_pasazera;
            resp.numer_kasy = numer_kasy;
            //2% szans na odmowe sprzedazy z powodu braku srodkow pasazera
            if (losuj(1, 100) <= 2) {
                resp.sukces = 0;
                resp.brak_srodkow = 1;
                int retry = 0;
                while (msgsnd(msg_kasa_id, &resp, sizeof(KasaResponse) - sizeof(long), IPC_NOWAIT) == -1 && retry < 5) {
                    if (errno == EAGAIN) { 
                        //usleep(5000); 
                        retry++; continue; }
                    break;
                }
                log_print(KOLOR_KASA, tag, "Odmowa sprzedazy PAS %d - BRAK SRODKOW", req.id_pasazera);
                continue;
            }
            //aktualizacja statystyk
            while (semop(sem_id, &shm_lock, 1) == -1) {
                if (errno == EINTR) continue;
                break;
            }
            if (shm->registered_count < MAX_REGISTERED) {
                shm->registered_pids[shm->registered_count] = req.pid_pasazera;
                shm->registered_wiek[shm->registered_count] = req.wiek;
                shm->registered_count++;
            }
            shm->sprzedanych_biletow += req.ile_biletow;
            shm->obsluzonych_kasa[numer_kasy - 1]++;
            while (semop(sem_id, &shm_unlock, 1) == -1 && errno == EINTR);
            resp.sukces = 1;
            int retry = 0;
            while (msgsnd(msg_kasa_id, &resp, sizeof(KasaResponse) - sizeof(long), IPC_NOWAIT) == -1 && retry < 5) {
                if (errno == EAGAIN) { 
                    //usleep(5000); 
                    retry++; continue; }
                break;
            }
            log_print(KOLOR_KASA, tag, "Sprzedano %d bilet(y) PAS %d",
                      req.ile_biletow, req.id_pasazera);
            obsluzonych++;
        } else {
            //usleep(5000);  //sched_yield();
        }
    }//Przed zamknieciem odrzuca wszystkich czekajacych w kolejce
    {
        KasaRequest req;
        while (msgrcv(msg_kasa_id, &req, sizeof(KasaRequest) - sizeof(long), 
                      numer_kasy, IPC_NOWAIT) != -1) {
            KasaResponse resp;
            resp.mtype = req.pid_pasazera;
            resp.numer_kasy = numer_kasy;
            resp.sukces = 0;
            resp.brak_srodkow = 0;
            msgsnd(msg_kasa_id, &resp, sizeof(KasaResponse) - sizeof(long), IPC_NOWAIT);
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
    if (init_ipc_client() == -1) exit(1);//inicjalizacja IPC dla procesu potomnego
    int numer_kasy = atoi(argv[1]);
    if (numer_kasy <= 0 || numer_kasy > MAX_KASY) { 
        fprintf(stderr, "Blad: numer_kasy musi byc 1-%d\n", MAX_KASY); 
        exit(1); 
    }
    proces_kasa(numer_kasy);//start procesu kasy
    return 0;
}