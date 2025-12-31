// dyspozytor.c - Glowny modul zarzadzajacy symulacja
#include "common.h"
#include "dyspozytor.h"
#include "pasazer.h"
#include "bus.h"
#include "kasa.h"


// PID-y procesow potomnych
static pid_t pid_kasa = -1;
static pid_t pids_busy[MAX_BUSES];
static int ile_busow = 0;
static pid_t pid_generator = -1;

// Flagi sygnalow
static volatile sig_atomic_t flaga_sigusr1 = 0;
static volatile sig_atomic_t flaga_sigusr2 = 0;
static volatile sig_atomic_t flaga_stop = 0;

// Handler sygnalow dla dyspozytora (WYMAGANIE 5.2.d: sigaction())
static void handler_dyspozytor(int sig) {
    if (sig == SIGUSR1) {
        flaga_sigusr1 = 1;
    } else if (sig == SIGUSR2) {
        flaga_sigusr2 = 1;
    } else if (sig == SIGINT || sig == SIGTERM) {
        flaga_stop = 1;
    }
}

// Inicjalizacja semaforow (WYMAGANIE 5.2.e: semget(), semctl())
static void init_semafory(int K) {
    union semun { int val; } arg;
    arg.val = 1;
    semctl(sem_id, SEM_DOOR_NORMAL, SETVAL, arg);
    semctl(sem_id, SEM_DOOR_ROWER, SETVAL, arg);
    semctl(sem_id, SEM_BUS_STOP, SETVAL, arg);
    semctl(sem_id, SEM_LOG, SETVAL, arg);
    semctl(sem_id, SEM_SHM, SETVAL, arg);
    
    for (int i = 0; i < K; i++) {
        semctl(sem_id, SEM_KASA_BASE + i, SETVAL, arg);
    }
}

// Sprzatanie zasobow IPC
static void cleanup_ipc(void) {
    if (sem_id != -1) semctl(sem_id, 0, IPC_RMID);
    if (shm_id != -1) shmctl(shm_id, IPC_RMID, NULL);
    if (msg_id != -1) msgctl(msg_id, IPC_RMID, NULL);
}

// Zamkniecie wszystkich procesow potomnych
static void shutdown_children(void) {
    /* Generator */
    if (pid_generator > 0) {
        kill(pid_generator, SIGTERM);
    }

    /* Kasa */
    if (pid_kasa > 0) {
        kill(pid_kasa, SIGTERM);
    }

    /* Autobusy */
    for (int i = 0; i < ile_busow; i++) {
        if (pids_busy[i] > 0) {
            kill(pids_busy[i], SIGTERM);
        }
    }

    /* Czekaj na zakończenie */
    while (wait(NULL) > 0);
}

// proces_dyspozytor - Glowna funkcja dyspozytora
void proces_dyspozytor(int N, int P, int R, int T, int K) {
    srand(time(NULL) ^ getpid());

    // KONFIGURACJA SYGNALOW (WYMAGANIE 5.2.d: sigaction())
    // Signal
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handler_dyspozytor;
    sa.sa_flags = SA_RESTART;
    sigemptyset(&sa.sa_mask);

    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGUSR2, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    // TWORZENIE ZASOBOW IPC (WYMAGANIE 5.2.e,g,h: ftok(), semget(), shmget(), msgget())
    key_t key_sem = ftok(".", 'S');
    key_t key_shm = ftok(".", 'M');
    key_t key_msg = ftok(".", 'Q');

    if (key_sem == -1 || key_shm == -1 || key_msg == -1) {
        perror("ftok");
        exit(1);
    }

    int sem_count = SEM_COUNT_BASE + K;
    sem_id = semget(key_sem, sem_count, IPC_CREAT | 0600);    shm_id = shmget(key_shm, sizeof(SharedData), IPC_CREAT | 0600);
    msg_id = msgget(key_msg, IPC_CREAT | 0600);

    if (sem_id == -1 || shm_id == -1 || msg_id == -1) {
        perror("IPC create");
        cleanup_ipc();
        exit(1);
    }

    init_semafory(K);

    // INICJALIZACJA PAMIECI DZIELONEJ (WYMAGANIE 5.2.g: shmat(), shmdt())
    SharedData *shm = (SharedData *)shmat(shm_id, NULL, 0);
    if (shm == (void *)-1) {
        perror("shmat");
        cleanup_ipc();
        exit(1);
    }

    memset(shm, 0, sizeof(SharedData));
    shm->param_N = N;
    shm->param_P = P;
    shm->param_R = R;
    shm->param_T = T;
    shm->param_K = K;
    shm->stacja_otwarta = true;
    shm->symulacja_aktywna = true;
    shm->bus_na_peronie = false;
    shm->aktualny_bus_pid = 0;

    shmdt(shm);

    // CZYSZCZENIE PLIKU RAPORTU (WYMAGANIE 5.2.a: creat())
    int fd = creat("raport.txt", 0644);
    if (fd != -1) {
        const char* header = "=== RAPORT SYMULACJI AUTOBUSU PODMIEJSKIEGO ===\n\n";
        write(fd, header, strlen(header));
        close(fd);
    }

    // START
    log_print(KOLOR_MAIN, "MAIN", "START: N=%d P=%d R=%d T=%d K=%d", N, P, R, T, K);    log_print(KOLOR_DYSP, "DYSP", "Dyspozytor rozpoczął pracę. PID=%d", getpid());
    log_print(KOLOR_DYSP, "DYSP", ">>> Sygnały: kill -SIGUSR1 %d | kill -SIGUSR2 %d <<<", 
              getpid(), getpid());

    // URUCHOMIENIE KASY (WYMAGANIE 5.2.b: fork())
    pid_kasa = fork();
    if (pid_kasa == -1) { perror("fork kasa"); cleanup_ipc(); exit(1); }
    if (pid_kasa == 0) {
        char arg_k[16];
        snprintf(arg_k, sizeof(arg_k), "%d", K);
        execl("./bin/kasa", "kasa", arg_k, NULL);
        perror("execl kasa"); exit(1);
    }
    log_print(KOLOR_KASA, "KASA", "Uruchomiono (K=%d watkow). PID=%d", K, pid_kasa);

    // URUCHOMIENIE AUTOBUSOW
    ile_busow = N;
    for (int i = 0; i < N; i++) {
        pids_busy[i] = fork();
        if (pids_busy[i] == -1) {
            perror("fork bus");
            continue;
        }
        if (pids_busy[i] == 0) {
            char arg_id[16], arg_p[16], arg_r[16], arg_t[16];
            snprintf(arg_id, sizeof(arg_id), "%d", i + 1);
            snprintf(arg_p, sizeof(arg_p), "%d", P);
            snprintf(arg_r, sizeof(arg_r), "%d", R);
            snprintf(arg_t, sizeof(arg_t), "%d", T);
            execl("./bin/autobus", "autobus", arg_id, arg_p, arg_r, arg_t, NULL);
            perror("execl autobus"); exit(1);
        }
        log_print(KOLOR_BUS, "BUS", "BUS %d uruchomiony. PID=%d", i+1, pids_busy[i]);
    }

    log_print(KOLOR_MAIN, "MAIN", "Wszystkie procesy uruchomione.");

    // URUCHOMIENIE GENERATORA PASAZEROW
    pid_generator = fork();
    if (pid_generator == -1) {
        perror("fork generator");
    } else if (pid_generator == 0) {
        /* Proces potomny - generator */
        srand(time(NULL) ^ getpid());
        int id_pas = 0;

        while (1) {
            SharedData *s = (SharedData *)shmat(shm_id, NULL, 0);
            if (s == (void *)-1) exit(1);

            bool aktywna = s->symulacja_aktywna;
            bool otwarta = s->stacja_otwarta;
            shmdt(s);

            if (!aktywna) break;
            if (!otwarta) {
                msleep(500);
                continue;
            }

            // Tworzenie nowego pasazera
            pid_t pas = fork();
            if (pas == 0) {
                char arg_id[16];
                snprintf(arg_id, sizeof(arg_id), "%d", id_pas);
                execl("./bin/pasazer", "pasazer", "normal", arg_id, NULL);
                exit(1);
            }
            id_pas++;

            // Losowy interval miedzy pasazerami
            msleep(losuj(800, 2000));
        }
        exit(0);
    }

    // GLOWNA PETLA DYSPOZYTORA
    while (!flaga_stop) {
        // Obsluga SIGUSR1 - wymuszony odjazd
        if (flaga_sigusr1) {
            flaga_sigusr1 = 0;
            
            SharedData *s = (SharedData *)shmat(shm_id, NULL, 0);
            if (s != (void *)-1) {
                if (s->aktualny_bus_pid > 0) {
                    log_print(KOLOR_DYSP, "DYSP", 
                              ">>> SIGUSR1: Wymuszam odjazd BUS PID=%d <<<", 
                              s->aktualny_bus_pid);
                    /* WYMAGANIE 5.2.d: kill() */
                    kill(s->aktualny_bus_pid, SIGUSR1);
                } else {
                    log_print(KOLOR_DYSP, "DYSP", "SIGUSR1: Brak autobusu na peronie.");
                }
                shmdt(s);
            }
        }

        // Obsluga SIGUSR2 - zamkniecie dworca
        if (flaga_sigusr2) {
            flaga_sigusr2 = 0;
            
            log_print(KOLOR_DYSP, "DYSP", ">>> SIGUSR2: ZAMYKAM DWORZEC! <<<");
            
            SharedData *s = (SharedData *)shmat(shm_id, NULL, 0);
            if (s != (void *)-1) {
                s->stacja_otwarta = false;  /* Nowi pasażerowie nie wchodzą */
                shmdt(s);
            }

            /* Zatrzymaj generator */
            if (pid_generator > 0) {
                kill(pid_generator, SIGTERM);
            }

            log_print(KOLOR_DYSP, "DYSP", 
                      "Dworzec zamknięty. Czekam na zakończenie przejazdów...");
        }

        // Sprawdzenie czy symulacja powinna sie zakonczyc
        SharedData *s = (SharedData *)shmat(shm_id, NULL, 0);
        if (s != (void *)-1) {
            // Koniec gdy stacja zamknieta i wszyscy w trasie rozwiezieni
            if (!s->stacja_otwarta && s->pasazerow_w_trasie <= 0) {
                log_print(KOLOR_DYSP, "DYSP", "Dworzec zamkniety, wszyscy rozwiezieni - koncze.");
                s->symulacja_aktywna = false;
                shmdt(s);
                break;
            }
            shmdt(s);
        }

        // Sprzatanie procesow zombie
        waitpid(-1, NULL, WNOHANG);

        usleep(200000);  // 200ms tick
    }

    // ZAKONCZENIE
    log_print(KOLOR_DYSP, "DYSP", "Zamykanie symulacji...");

    // Oznacz koniec symulacji
    SharedData *s = (SharedData *)shmat(shm_id, NULL, 0);
    if (s != (void *)-1) {
        s->symulacja_aktywna = false;
        s->stacja_otwarta = false;
        
        log_print(KOLOR_MAIN, "MAIN", "=== PODSUMOWANIE ===");
        log_print(KOLOR_MAIN, "MAIN", "Pasażerów wygenerowanych: %d", s->total_pasazerow);
        log_print(KOLOR_MAIN, "MAIN", "Biletów sprzedanych: %d", s->sprzedanych_biletow);
        log_print(KOLOR_MAIN, "MAIN", "Pasażerów zarejestrowanych: %d", s->registered_count);
        
        shmdt(s);
    }

    shutdown_children();
    cleanup_ipc();

    log_print(KOLOR_MAIN, "MAIN", "Symulacja zakończona.");
}
