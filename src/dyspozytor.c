//Glowny modul zarzadzajacy symulacja
//Tworzenie zasobów IPC (semafory, pamięć dzielona, kolejki)
//Uruchamianie procesów: kasa, autobusy, generator pasażerów
//Obsługę sygnałów SIGUSR1 (wymuszony odjazd) i SIGUSR2 (zamknięcie dworca)
//Graceful shutdown i cleanup zasobów
#include "common.h"
#include "dyspozytor.h"

//PID-y procesow potomnych
static pid_t pids_kasy[MAX_KASY]; //PID-y kas biletowych
static int ile_kas = 0;//liczba kas
static pid_t pids_busy[MAX_BUSES];//PID-y autobusow
static int ile_busow = 0; //liczba autobusow
static pid_t pid_generator = -1; //PID generatora pasazerow

//flagi sygnalow 
static volatile sig_atomic_t flaga_sigusr1 = 0;//SIGUSR1 = wymuszony odjazd autobusu
static volatile sig_atomic_t flaga_sigusr2 = 0;//SIGUSR2 = zamknięcie dworca
static volatile sig_atomic_t flaga_stop = 0;//SIGINT/SIGTERM = natychmiastowe zakończenie


//Handler sygnalow dla dyspozytora sigaction()
//Handler sygnalow - ustawia flagi, nie wykonuje operacji blokujących
static void handler_dyspozytor(int sig) {
    if (sig == SIGUSR1) {//SIGUSR1 = wymuszony odjazd autobusu z peronu
        flaga_sigusr1 = 1;
    } else if (sig == SIGUSR2) {//SIGUSR2 = zamknięcie dworca(graceful shutdown)
        flaga_sigusr2 = 1;
    } else if (sig == SIGINT || sig == SIGTERM || sig == SIGQUIT) {//SIGINT/SIGTERM/SIGQUIT = natychmiastowe zakonczenie
        flaga_stop = 1;
    }
}
//Handler SIGCHLD zbiera zombie automatycznie
static void handler_sigchld(int sig) {
    (void)sig;
    while (waitpid(-1, NULL, WNOHANG) > 0);//Zbieraj wszystkie zakończone procesy
}

//Inicjalizacja semaforow semget(), semctl()
//Wszystkie ustawione na 1 (mutex/binary semaphore)
static void init_semafory(void) {
    union semun { int val; } arg;
    arg.val = 1;
    semctl(sem_id, SEM_DOOR_NORMAL, SETVAL, arg);// SEM_DOOR_NORMAL/ROWER - kontrola wejścia do autobusu
    semctl(sem_id, SEM_DOOR_ROWER, SETVAL, arg);
    semctl(sem_id, SEM_BUS_STOP, SETVAL, arg);//SEM_BUS_STOP - tylko jeden autobus na peronie
    semctl(sem_id, SEM_LOG, SETVAL, arg);//SEM_LOG - sekcja krytyczna logow
    semctl(sem_id, SEM_SHM, SETVAL, arg);// SEM_SHM - ochrona pamięci dzielonej
}
//Sprzatanie zasobow IPC
//wywolywane przy zakonczeniu symulacji
static void cleanup_ipc(void) {
    if (sem_id != -1) semctl(sem_id, 0, IPC_RMID);
    if (shm_id != -1) shmctl(shm_id, IPC_RMID, NULL);
    if (msg_id != -1) msgctl(msg_id, IPC_RMID, NULL);
    if (msg_kasa_id != -1) msgctl(msg_kasa_id, IPC_RMID, NULL);
}
//zamkniecie wszystkich procesow potomnych
//Wysyla SIGTERM i czeka na zakonczenie
static void shutdown_children(void) {
    //Generator 
    if (pid_generator > 0) {
        kill(pid_generator, SIGTERM);
    }
    //Kasy
    for (int i = 0; i < ile_kas; i++) {
        if (pids_kasy[i] > 0) {
            kill(pids_kasy[i], SIGTERM);
        }
    }
    //Autobusy 
    for (int i = 0; i < ile_busow; i++) {
        if (pids_busy[i] > 0) {
            kill(pids_busy[i], SIGTERM);
        }
    }
    //czekaj na zakonczenie wszystkich procesow potomnych 
    time_t start = time(NULL);
    while (time(NULL) - start < 2) {
        if (waitpid(-1, NULL, WNOHANG) <= 0) break;
    }
    //Jesli nadal dzialaja wymus SIGKILL
    if (pid_generator > 0) kill(pid_generator, SIGKILL);
    for (int i = 0; i < ile_kas; i++) {
        if (pids_kasy[i] > 0) kill(pids_kasy[i], SIGKILL);
    }
    for (int i = 0; i < ile_busow; i++) {
        if (pids_busy[i] > 0) kill(pids_busy[i], SIGKILL);
    }
    //ostateczne czekanie
    while (waitpid(-1, NULL, WNOHANG) > 0);
}
//zapisz raport koncowy do pliku
static void zapisz_raport_koncowy(SharedData *shm) {
    int fd = open("raport.txt", O_WRONLY | O_CREAT | O_APPEND, 0644);
    if(fd == -1) return;
    char buf[2048];
    int len = snprintf(buf, sizeof(buf),
        "\n========================================\n"
        "       RAPORT KONCOWY SYMULACJI\n"
        "========================================\n"
        "Pasazerow ogolem: %d\n"
        "Przewiezionych: %d\n"
        "Opuscilo bez jazdy: %d\n"
        "----------------------------------------\n"
        "Biletow sprzedanych: %d\n"
        "VIP: %d\n"
        "Rodzicow z dziecmi: %d (par)\n"
        "Odrzuconych bez biletu: %d\n"
        "========================================\n",
        shm->total_pasazerow, shm->total_przewiezionych,
        shm->opuscilo_bez_jazdy,
        shm->sprzedanych_biletow, shm->vip_count, 
        shm->rodzicow_z_dziecmi, shm->odrzuconych_bez_biletu);
    write(fd, buf, len);

    for(int i = 0; i < shm->param_K; i++) {
        len = snprintf(buf, sizeof(buf), "KASA %d obsluzyla: %d pasazerow\n", 
                       i + 1, shm->obsluzonych_kasa[i]);
        write(fd, buf, len);
    }
    close(fd);
}
//proces_dyspozytor - Glowna funkcja dyspozytora
void proces_dyspozytor(int N, int P, int R, int T, int K) {
    //Automatyczne zbieranie zombie przez kernel
    signal(SIGCHLD, SIG_IGN);
    srand(time(NULL) ^ getpid());

    //Konfiguracja sygnalow
    struct sigaction sa;
    memset(&sa, 0,sizeof(sa));
    sa.sa_handler =handler_dyspozytor;
    sa.sa_flags =SA_RESTART;
    sigemptyset(&sa.sa_mask);

    sigaction(SIGUSR1, &sa,NULL);
    sigaction(SIGUSR2, &sa,NULL);
    sigaction(SIGINT,  &sa,NULL);
    sigaction(SIGTERM, &sa,NULL);
    sigaction(SIGQUIT, &sa,NULL); 
    
    //Handler SIGCHLD automatyczne zbieranie zombie
    struct sigaction sa_chld;
    memset(&sa_chld, 0, sizeof(sa_chld));
    sa_chld.sa_handler =handler_sigchld;
    sa_chld.sa_flags =SA_RESTART |SA_NOCLDSTOP;
    sigemptyset(&sa_chld.sa_mask);
    sigaction(SIGCHLD,&sa_chld, NULL);

    //tworzenie zasobow IPC ftok(), semget(), shmget(), msgget()
    key_t key_sem =ftok(".", 'S');
    key_t key_shm =ftok(".", 'M');
    key_t key_msg =ftok(".", 'Q');
    key_t key_msg_kasa =ftok(".", 'K');

    if (key_sem == -1 || key_shm == -1 || key_msg == -1 || key_msg_kasa == -1) {
        perror("ftok");
        exit(1);
    }
    sem_id = semget(key_sem, SEM_COUNT, IPC_CREAT | 0600);
    shm_id = shmget(key_shm, sizeof(SharedData), IPC_CREAT | 0600);
    msg_id = msgget(key_msg, IPC_CREAT | 0600);
    msg_kasa_id = msgget(key_msg_kasa, IPC_CREAT | 0600);

    if (sem_id == -1 || shm_id == -1 || msg_id == -1 || msg_kasa_id == -1) {
        perror("IPC create");
        cleanup_ipc();
        exit(1);
    }
    //zwiekszenie limitu kolejek komunikatow
    struct msqid_ds buf;
    //Kolejka autobus-pasazer
    if (msgctl(msg_id, IPC_STAT, &buf) == 0) {
        buf.msg_qbytes = 65536; 
        msgctl(msg_id, IPC_SET, &buf);
    }
    //Kolejka kasa-pasazer
    if (msgctl(msg_kasa_id, IPC_STAT, &buf) == 0) {
        buf.msg_qbytes = 1048576;  //1MB  
        msgctl(msg_kasa_id, IPC_SET, &buf);
    }
    init_semafory();
    //inicjalizacja pamieci dzielonej shmat() shmdt()
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
    shm->dworzec_otwarty = true;
    shm->symulacja_aktywna = true;
    shm->bus_na_przystanku = false;
    shm->aktualny_bus_pid = 0;

    shmdt(shm);
    //czyszczenie pliku raportu creat()
    int fd = creat("raport.txt", 0644);
    if (fd != -1) {
        const char* header = "=== RAPORT SYMULACJI AUTOBUSU PODMIEJSKIEGO ===\n\n";
        write(fd, header, strlen(header));
        close(fd);
    }
    //START
    log_print(KOLOR_MAIN, "MAIN", "START: N=%d P=%d R=%d T=%d K=%d", N, P, R, T, K);
    log_print(KOLOR_DYSP, "DYSP", "Dyspozytor rozpoczął pracę. PID=%d", getpid());
    log_print(KOLOR_DYSP, "DYSP", ">>> Sygnały: kill -SIGUSR1 %d | kill -SIGUSR2 %d <<<", 
              getpid(), getpid());
    //uruchomienie K kas
    ile_kas = K;
    for (int i = 0; i < K; i++) {
        pids_kasy[i] = fork();
        if (pids_kasy[i] == -1) { 
            perror("fork kasa"); 
            shutdown_children();
            cleanup_ipc();
            exit(1);
        }
        if (pids_kasy[i] == 0) {
            char arg_numer[16];
            snprintf(arg_numer, sizeof(arg_numer), "%d", i + 1);
            execl("./bin/kasa", "kasa", arg_numer, NULL);
            perror("execl kasa"); exit(1);
        }
        log_print(KOLOR_KASA, "KASA", "KASA %d uruchomiona. PID=%d", i + 1, pids_kasy[i]);
    }
    //uruchomienie autobusow
    ile_busow = N;
    for (int i = 0; i < N; i++) {
        pids_busy[i] = fork();
        if (pids_busy[i] == -1) {
            perror("fork bus");
            shutdown_children();
            cleanup_ipc();
            exit(1);
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
    
    //uruchomienie generatora pasazerow (przez exec)
    pid_generator = fork();
    if (pid_generator == -1) {
        perror("fork generator");
        shutdown_children();
        cleanup_ipc();
        exit(1);
    } else if (pid_generator == 0) {
        execl("./bin/pasazer", "pasazer", "generator", NULL);
        perror("execl generator");
        exit(1);
    }
    //GŁÓWNA PETLA DYSPOTYZORA
    while (!flaga_stop) {
        //obsluga SIGUSR1 wymuszony odjazd
        if (flaga_sigusr1) {
            flaga_sigusr1 = 0;
            SharedData *s = (SharedData *)shmat(shm_id, NULL, 0);
            if (s != (void *)-1) {
                //Sprawdz OBYDWA warunki: bus_na_przystanku ORAZ aktualny_bus_pid
                if (s->bus_na_przystanku && s->aktualny_bus_pid > 0) {
                    log_print(KOLOR_DYSP, "DYSP", 
                              ">>> SIGUSR1: Wymuszam odjazd BUS PID=%d <<<", 
                              s->aktualny_bus_pid);
                    kill(s->aktualny_bus_pid, SIGUSR1);
                } else {
                    log_print(KOLOR_DYSP, "DYSP", "SIGUSR1: Brak autobusu na peronie.");
                }
                shmdt(s);
            }
        }
        //obsluga SIGUSR2 zamkniecie dworca
        if (flaga_sigusr2) {
            flaga_sigusr2 = 0;
            
            log_print(KOLOR_DYSP, "DYSP", ">>> SIGUSR2: ZAMYKAM DWORZEC! <<<");
            SharedData *s = (SharedData *)shmat(shm_id, NULL, 0);
            if (s != (void *)-1) {
                s->dworzec_otwarty = false;//nowi pasazerowie nie wchodza na dworzec
                shmdt(s);
            }
            //zatrzymaj generator
            if (pid_generator > 0) {
                kill(pid_generator, SIGTERM);
            }
            //zatrzymaj kasy
            for (int i = 0; i < ile_kas; i++) {
                if (pids_kasy[i] > 0) {
                    kill(pids_kasy[i], SIGTERM);
                }
            }//Wyczysc kolejke kasy - wyslij odmowy wszystkim czekajacym
            {
                KasaRequest req;
                KasaResponse resp;
                resp.sukces = 0;
                resp.brak_srodkow = 0;
                while (msgrcv(msg_kasa_id, &req, sizeof(KasaRequest) - sizeof(long), 0, IPC_NOWAIT) != -1) {
                    resp.mtype = req.pid_pasazera;
                    resp.numer_kasy = 0;
                    msgsnd(msg_kasa_id, &resp, sizeof(KasaResponse) - sizeof(long), IPC_NOWAIT);
                }
            }
            log_print(KOLOR_DYSP, "DYSP", "Dworzec zamkniety. Czekam na zakonczenie przejazdow");
        }
        //sprawdzenie czy symulacja powinna sie zakonczyc
        static time_t czas_zamkniecia = 0;
        SharedData *s = (SharedData *)shmat(shm_id, NULL, 0);
        if (s != (void *)-1) {
            if (!s->dworzec_otwarty) {
                //Ustaw czas zamkniecia przy pierwszym wykryciu
                if (czas_zamkniecia == 0) {
                    czas_zamkniecia = time(NULL);
                }//Log co blokuje
                if ((time(NULL) - czas_zamkniecia) % 10 == 0) {//co 10 sekund
                    log_print(KOLOR_DYSP, "DYSP", "Czekam: w trasie=%d, czeka=%d", 
                              s->pasazerow_w_trasie, s->pasazerow_czeka);
                }//Normalne zakończenie
                if (s->pasazerow_w_trasie <= 0 && s->pasazerow_czeka <= 0) {
                    log_print(KOLOR_DYSP, "DYSP", "Dworzec zamkniety, wszyscy rozwiezieni - koncze.");
                    s->symulacja_aktywna = false;
                    shmdt(s);
                    break;
                }//TIMEOUT 60 sekund - wymuś zakończenie
                if (time(NULL) - czas_zamkniecia > 60) {
                    log_print(KOLOR_DYSP, "DYSP", "TIMEOUT 60s - wymuszam zakonczenie! w_trasie=%d, czeka=%d",
                              s->pasazerow_w_trasie, s->pasazerow_czeka);
                    s->symulacja_aktywna = false;
                    s->pasazerow_w_trasie = 0;
                    s->pasazerow_czeka = 0;
                    shmdt(s);
                    break;
                }
            }shmdt(s);
        }
        waitpid(-1, NULL, WNOHANG);
        usleep(100000);
    }
    //ZAKONCZENIE
    if (flaga_stop) {
        log_print(KOLOR_DYSP, "DYSP", ">>> NATYCHMIASTOWE ZAKONCZENIE! <<<");
        SharedData *s = (SharedData *)shmat(shm_id, NULL, 0);
        if (s != (void *)-1) {
            int pasazerow_na_dworcu = s->pasazerow_czeka;
            int pasazerow_w_trasie = s->pasazerow_w_trasie;
            s->symulacja_aktywna = false;
            s->dworzec_otwarty = false;
            //pasazerowie na dworcu
            s->opuscilo_bez_jazdy += pasazerow_na_dworcu;
            s->pasazerow_czeka = 0;
            //Pasazerowie w trasie 
            s->total_przewiezionych += pasazerow_w_trasie;
            s->pasazerow_w_trasie = 0;
            shmdt(s);
            log_print(KOLOR_DYSP, "DYSP", "Pasazerow na dworcu: %d", pasazerow_na_dworcu);
            if (pasazerow_w_trasie > 0) {
                log_print(KOLOR_DYSP, "DYSP", "Pasazerow w trasie: %d", pasazerow_w_trasie);
            }
        }
        //usleep(2000); //lub sched_yield();
        //zabij wszystkie procesy potomne
        shutdown_children();
    } else {
        //Graceful shutdown (SIGUSR2) - normalne zakonczenie
        //Petla glowna juz ustawila dworzec_otwarty=false i czekala na pasazerow
        log_print(KOLOR_DYSP, "DYSP", "Zamykanie symulacji (SIGUSR2)");
        SharedData *s = (SharedData *)shmat(shm_id, NULL, 0);
        if (s != (void *)-1) {
            s->symulacja_aktywna = false;
            shmdt(s);
        }
        log_print(KOLOR_DYSP, "DYSP", "Czekam na zakonczenie procesow");
        {
            time_t wait_start = time(NULL);
            while (time(NULL) < wait_start + 10) {
                waitpid(-1, NULL, WNOHANG);
            }
        }
        //usleep(2000); //lub sched_yield();
        shutdown_children();
    }
    //Podsumowanie - TYLKO przy SIGUSR2 (graceful shutdown)
    if (!flaga_stop) {
        SharedData *s = (SharedData *)shmat(shm_id, NULL, 0);
        if (s != (void *)-1) {
            log_print(KOLOR_STAT, "STAT", "========================================");
            log_print(KOLOR_STAT, "STAT", "PODSUMOWANIE");
            log_print(KOLOR_STAT, "STAT", "========================================");
            log_print(KOLOR_STAT, "STAT", "Pasazerow ogolem: %d", s->total_pasazerow);
            log_print(KOLOR_STAT, "STAT", "Przewiezionych: %d", s->total_przewiezionych);
            log_print(KOLOR_STAT, "STAT", "Opuscilo bez jazdy: %d", s->opuscilo_bez_jazdy);
            log_print(KOLOR_STAT, "STAT", "----------------------------------------");
            log_print(KOLOR_STAT, "STAT", "Biletow: %d", s->sprzedanych_biletow);
            log_print(KOLOR_STAT, "STAT", "VIP: %d", s->vip_count);
            log_print(KOLOR_STAT, "STAT", "Rodzicow z dziecmi: %d (par)", s->rodzicow_z_dziecmi);
            log_print(KOLOR_STAT, "STAT", "Odrzuconych bez biletu: %d", s->odrzuconych_bez_biletu);
            for (int i = 0; i < s->param_K; i++) {
                log_print(KOLOR_STAT, "STAT", "KASA %d: %d", i+1, s->obsluzonych_kasa[i]);
            }
            log_print(KOLOR_STAT, "STAT", "========================================");
            zapisz_raport_koncowy(s);
            shmdt(s);
        }
    }
    cleanup_ipc();
    log_print(KOLOR_MAIN, "MAIN", "Koniec.");
}
