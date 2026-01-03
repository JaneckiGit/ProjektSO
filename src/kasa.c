// kasa.c - Modul kasy biletowej (aktywna obsluga pasazerow)
#include "common.h"
#include "kasa.h"

static volatile int kasa_running = 1;
static pthread_mutex_t kasa_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t kasa_cond = PTHREAD_COND_INITIALIZER;

static void handler_kasa(int sig) {
    (void)sig;
    kasa_running = 0;
    pthread_cond_broadcast(&kasa_cond);
}

static void* kasa_watek(void* arg) {
    int numer_kasy = *(int*)arg;
    char tag[16];
    snprintf(tag, sizeof(tag), "KASA %d", numer_kasy);

    log_print(KOLOR_KASA, tag, "Otwarta. TID=%lu", (unsigned long)pthread_self());

    SharedData *shm = (SharedData *)shmat(shm_id, NULL, 0);
    if (shm == (void *)-1) { perror("kasa shmat"); return NULL; }

    struct sembuf shm_lock = {SEM_SHM, -1, SEM_UNDO};
    struct sembuf shm_unlock = {SEM_SHM, 1, SEM_UNDO};

    while (kasa_running && shm->symulacja_aktywna) {
        KasaRequest req;
        ssize_t ret = msgrcv(msg_kasa_id, &req, sizeof(KasaRequest) - sizeof(long), 
                             numer_kasy, IPC_NOWAIT);
        
        if (ret != -1) {
            pthread_mutex_lock(&kasa_mutex);
            
            log_print(KOLOR_KASA, tag, "Obsluguje PAS %d (wiek=%d, biletow=%d)", 
                      req.id_pasazera, req.wiek, req.ile_biletow);
            
            msleep(losuj(200, 500));
            
            semop(sem_id, &shm_lock, 1);
            if (shm->registered_count < MAX_REGISTERED) {
                shm->registered_pids[shm->registered_count] = req.pid_pasazera;
                shm->registered_wiek[shm->registered_count] = req.wiek;
                shm->registered_count++;
            }
            shm->sprzedanych_biletow += req.ile_biletow;
            shm->obsluzonych_kasa[numer_kasy - 1]++;
            semop(sem_id, &shm_unlock, 1);
            
            KasaResponse resp;
            resp.mtype = req.pid_pasazera;
            resp.numer_kasy = numer_kasy;
            resp.sukces = 1;
            msgsnd(msg_kasa_id, &resp, sizeof(KasaResponse) - sizeof(long), 0);
            
            log_print(KOLOR_KASA, tag, "Sprzedano %d bilet(y) PAS %d", 
                      req.ile_biletow, req.id_pasazera);
            
            pthread_cond_signal(&kasa_cond);
            pthread_mutex_unlock(&kasa_mutex);
        } else {
            pthread_mutex_lock(&kasa_mutex);
            struct timespec ts;
            clock_gettime(CLOCK_REALTIME, &ts);
            ts.tv_nsec += 50000000;
            if (ts.tv_nsec >= 1000000000) {
                ts.tv_sec += 1;
                ts.tv_nsec -= 1000000000;
            }
            pthread_cond_timedwait(&kasa_cond, &kasa_mutex, &ts);
            pthread_mutex_unlock(&kasa_mutex);
        }
    }

    int obsluzonych = shm->obsluzonych_kasa[numer_kasy - 1];
    log_print(KOLOR_KASA, tag, "Zamknieta. Obsluzono: %d. TID=%lu", 
              obsluzonych, (unsigned long)pthread_self());
    
    shmdt(shm);
    return NULL;
}

void proces_kasa(int K) {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handler_kasa;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    log_print(KOLOR_KASA, "KASA", "Uruchamiam %d kas (watkow). PID=%d", K, getpid());

    pthread_mutex_init(&kasa_mutex, NULL);
    pthread_cond_init(&kasa_cond, NULL);

    pthread_t* tidy = malloc(K * sizeof(pthread_t));
    int* numery = malloc(K * sizeof(int));

    for (int i = 0; i < K; i++) {
        numery[i] = i + 1;
        pthread_create(&tidy[i], NULL, kasa_watek, &numery[i]);
    }

    while (kasa_running) {
        SharedData *shm = (SharedData *)shmat(shm_id, NULL, 0);
        if (shm != (void *)-1) {
            if (!shm->symulacja_aktywna) { shmdt(shm); break; }
            shmdt(shm);
        }
        usleep(500000);
    }

    kasa_running = 0;
    pthread_cond_broadcast(&kasa_cond);
    
    for (int i = 0; i < K; i++) {
        pthread_join(tidy[i], NULL);
    }

    pthread_mutex_destroy(&kasa_mutex);
    pthread_cond_destroy(&kasa_cond);
    free(tidy);
    free(numery);
    
    log_print(KOLOR_KASA, "KASA", "Wszystkie kasy zamkniete. PID=%d", getpid());
    exit(0);
}

int main(int argc, char *argv[]) {
    if (argc < 2) { fprintf(stderr, "Uzycie: %s <liczba_kas>\n", argv[0]); exit(1); }
    if (init_ipc_client() == -1) exit(1);
    int K = atoi(argv[1]);
    if (K <= 0 || K > MAX_KASY) { fprintf(stderr, "Blad: K musi byc 1-%d\n", MAX_KASY); exit(1); }
    proces_kasa(K);
    return 0;
}