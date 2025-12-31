// kasa.c - Modul kasy biletowej
#include "common.h"
#include "kasa.h"

static volatile int kasa_running = 1;
static pthread_mutex_t kasa_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t kasa_cond = PTHREAD_COND_INITIALIZER;

typedef struct {
    int numer;
    pthread_t tid;
} KasaArg;

// Handler sygnalow kasy
static void handler_kasa(int sig) {
    (void)sig;
    kasa_running = 0;
    pthread_cond_broadcast(&kasa_cond);
}

// Watek kas
static void* kasa_watek(void* arg) {
    KasaArg* kasa = (KasaArg*)arg;
    char tag[16];
    snprintf(tag, sizeof(tag), "KASA %d", kasa->numer);

    log_print(KOLOR_KASA, tag, "Otwarta. TID=%lu", (unsigned long)pthread_self());

    while (kasa_running) {
        pthread_mutex_lock(&kasa_mutex);
        while (kasa_running) {
            pthread_cond_wait(&kasa_cond, &kasa_mutex);
            if (!kasa_running) break;
        }
        pthread_mutex_unlock(&kasa_mutex);
    }

    SharedData *shm = (SharedData *)shmat(shm_id, NULL, 0);
    if (shm != (void *)-1) {
        int obsluzonych = shm->obsluzonych_kasa[kasa->numer - 1];
        log_print(KOLOR_KASA, tag, "Zamknieta. Obsluzono: %d. TID=%lu", 
                  obsluzonych, (unsigned long)pthread_self());
        shmdt(shm);
    } else {
        log_print(KOLOR_KASA, tag, "Zamknieta. TID=%lu", (unsigned long)pthread_self());
    }

    return NULL;
}

// proces_kasa - Glowna funkcja kasy
void proces_kasa(int K) {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handler_kasa;
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    log_print(KOLOR_KASA, "KASA", "Uruchamiam %d kas (watkow). PID=%d", K, getpid());

    pthread_mutex_init(&kasa_mutex, NULL);
    pthread_cond_init(&kasa_cond, NULL);

    KasaArg* kasy = malloc(K * sizeof(KasaArg));
    if (!kasy) { perror("malloc kasy"); exit(1); }

    for (int i = 0; i < K; i++) {
        kasy[i].numer = i + 1;
        pthread_create(&kasy[i].tid, NULL, kasa_watek, &kasy[i]);
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
        pthread_join(kasy[i].tid, NULL);
    }

    pthread_mutex_destroy(&kasa_mutex);
    pthread_cond_destroy(&kasa_cond);
    free(kasy);

    log_print(KOLOR_KASA, "KASA", "Wszystkie kasy zamkniete. PID=%d", getpid());
    exit(0);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uzycie: %s <liczba_kas>\n", argv[0]);
        exit(1);
    }
    
    if (init_ipc_client() == -1) exit(1);
    
    int K = atoi(argv[1]);
    if (K <= 0 || K > MAX_KASY) {
        fprintf(stderr, "Blad: K musi byc 1-%d\n", MAX_KASY);
        exit(1);
    }
    
    proces_kasa(K);
    return 0;
}
