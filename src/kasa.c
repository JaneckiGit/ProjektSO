// kasa.c - Modul kasy biletowej
#include "common.h"
#include "kasa.h"

static volatile int kasa_running = 1;
static pthread_mutex_t kasa_mutex = PTHREAD_MUTEX_INITIALIZER;

// Struktura dla watku okienka
typedef struct {
    int numer;
    int obsluzonych;
} OkienkoArg;

// Handler sygnalow kasy
static void handler_kasa(int sig) {
    (void)sig;
    kasa_running = 0;
}

// Watek okienka kasowego
static void* okienko_func(void* arg) {
    OkienkoArg* okno = (OkienkoArg*)arg;
    char tag[16];
    snprintf(tag, sizeof(tag), "KASA");

    log_print(KOLOR_KASA, tag, "Okienko %d otwarte. TID=%lu",
              okno->numer, (unsigned long)pthread_self());

    while (kasa_running) {
        //Symulacja pracy okienka 
        pthread_mutex_lock(&kasa_mutex);
        okno->obsluzonych++;
        pthread_mutex_unlock(&kasa_mutex);

        usleep(500000);  
    }

    log_print(KOLOR_KASA, tag, "Okienko %d zamknięte. Obsłużono: %d",
              okno->numer, okno->obsluzonych);

    return NULL;
}

// proces_kasa - Glowna funkcja kasy
void proces_kasa(void) {
    // Konfiguracja sygnalow
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handler_kasa;
    sa.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    log_print(KOLOR_KASA, "KASA", "Otwarcie kasy. PID=%d", getpid());

    // Tworzenie wątków dla 2 okienek 
    pthread_t tid1, tid2;
    OkienkoArg okno1 = {1, 0};
    OkienkoArg okno2 = {2, 0};

    pthread_create(&tid1, NULL, okienko_func, &okno1);
    pthread_create(&tid2, NULL, okienko_func, &okno2);

    // Wątek główny - raportowanie 
    while (kasa_running) {
        SharedData *shm = (SharedData *)shmat(shm_id, NULL, 0);
        if (shm != (void *)-1) {
            if (!shm->symulacja_aktywna) {
                shmdt(shm);
                break;
            }
            shmdt(shm);
        }
        sleep(3);
    }

    kasa_running = 0;

    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);

    pthread_mutex_destroy(&kasa_mutex);

    // Wyświetl listę zarejestrowanych 
    SharedData *shm = (SharedData *)shmat(shm_id, NULL, 0);
    if (shm != (void *)-1) {
        log_print(KOLOR_KASA, "KASA", "=== ZAREJESTROWANI PASAŻEROWIE ===");
        for (int i = 0; i < shm->registered_count && i < 20; i++) {
            log_print(KOLOR_KASA, "KASA", "  %3d. PID=%d, Wiek=%d",
                      i+1, shm->registered_pids[i], shm->registered_wiek[i]);
        }
        if (shm->registered_count > 20) {
            log_print(KOLOR_KASA, "KASA", "  ... i %d więcej",
                      shm->registered_count - 20);
        }
        log_print(KOLOR_KASA, "KASA", "ŁĄCZNIE: %d pasażerów, %d biletów",
                  shm->registered_count, shm->sprzedanych_biletow);
        shmdt(shm);
    }

    log_print(KOLOR_KASA, "KASA", "Zamknięcie kasy. PID=%d", getpid());
    exit(0);
}
