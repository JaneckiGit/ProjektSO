// pasazer.c - Modul pasazera
//"generator" - tworzy nowych pasażerów co 800-2000ms
// "normal" - dorosły 9-80 lat, kupuje 1 bilet
//"dziecko" - dziecko 1-7 lat, czeka PRZED dworcem na opiekuna
//"rodzic" - opiekun 18-80 lat, zabiera dziecko, kupuje 2 bilety
//VIP (1% szans): omija kolejkę do kasy i ma priorytet w autobusie
//ROWER zajmuje miejsce w puli rowerowej autobusu
#include "common.h"
#include "pasazer.h"

// Struktura DzieckoWatekData zdefiniowana w common.h
// Wątek dziecka - czeka na rodzica, kończy gdy rodzic da sygnał
static void* watek_dziecko(void* arg) {
    DzieckoWatekData *d = (DzieckoWatekData *)arg;
    
    char tag[16];
    snprintf(tag, sizeof(tag), "PAS %d", d->id_dziecka);
    
    log_print(KOLOR_PAS, tag, "[WATEK] Dziecko (wiek=%d) przejete przez opiekuna. TID=%lu", 
              d->wiek_dziecka, (unsigned long)pthread_self());
    
    // Czekaj na sygnał zakończenia od rodzica
    pthread_mutex_lock(d->mutex);
    while (!(*d->zakoncz)) {
        pthread_cond_wait(d->cond, d->mutex);
    }
    pthread_mutex_unlock(d->mutex);
    
    log_print(KOLOR_PAS, tag, "[WATEK] Dziecko konczy swoje działanie. TID=%lu", 
              (unsigned long)pthread_self());
    
    return NULL;
}

// Wysyla bilet do autobusu przez kolejke komunikatow
static int wyslij_bilet(SharedData *shm, int id_pas, int wiek, int czy_rower, int czy_vip,
                        int ma_bilet, pid_t pid_dziecka, int id_dziecka, int wiek_dziecka) {
    BiletMsg bilet;
    memset(&bilet, 0, sizeof(bilet));
    
    bilet.mtype = czy_vip ? shm->aktualny_bus_pid : (shm->aktualny_bus_pid + 1000000);
    bilet.pid_pasazera = getpid();
    bilet.id_pasazera = id_pas;
    bilet.wiek = wiek;
    bilet.czy_rower = czy_rower;
    bilet.czy_vip = czy_vip;
    bilet.ma_bilet = ma_bilet;
    bilet.pid_dziecka = pid_dziecka;
    bilet.id_dziecka = id_dziecka;
    bilet.wiek_dziecka = wiek_dziecka;

    return msgsnd(msg_id, &bilet, sizeof(BiletMsg) - sizeof(long), 0);
}

// Glowna petla oczekiwania na autobus i wsiadania
static int czekaj_na_autobus(SharedData *shm, const char *tag, int id_pas, int wiek, 
                              int czy_rower, int czy_vip, int ma_bilet,
                              pid_t pid_dziecka, int id_dziecka, int wiek_dziecka,
                              int ile_osob) {

    struct sembuf shm_lock = {SEM_SHM, -1, SEM_UNDO};
    struct sembuf shm_unlock = {SEM_SHM, 1, SEM_UNDO};  
    int sem_drzwi = czy_rower ? SEM_DOOR_ROWER : SEM_DOOR_NORMAL;
    
    // PID autobusu który odmówił - nie próbuj ponownie do tego samego
    pid_t bus_ktory_odmowil = 0;
    // Flaga: czy czekamy na odpowiedź od autobusu (zapobiega podwójnemu wysłaniu)
    int czekam_na_odpowiedz = 0;
    pid_t bus_do_ktorego_wyslalem = 0;

    while (shm->symulacja_aktywna) {
        // SIGUSR2 - dworzec zamkniety
        if (!shm->stacja_otwarta) {
            log_print(KOLOR_PAS, tag, "Dworzec zamkniety - opuszczam. PID=%d", getpid());
            semop(sem_id, &shm_lock, 1);
            shm->pasazerow_czeka -= ile_osob;
            semop(sem_id, &shm_unlock, 1);
            return -1;
        }

        if (shm->bus_na_peronie && shm->aktualny_bus_pid > 0) {
            pid_t aktualny_bus = shm->aktualny_bus_pid;
            
            // Jeśli to ten sam autobus który odmówił - czekaj aż odjedzie
            if (aktualny_bus == bus_ktory_odmowil) {
                usleep(200000);  // 200ms - czekaj aż odjedzie
                continue;
            }
            
            // Jeśli czekamy na odpowiedź ale autobus się zmienił - reset
            if (czekam_na_odpowiedz && aktualny_bus != bus_do_ktorego_wyslalem) {
                czekam_na_odpowiedz = 0;
                bus_do_ktorego_wyslalem = 0;
            }
            
            // Wysyłaj bilet TYLKO jeśli nie czekamy już na odpowiedź
            if (!czekam_na_odpowiedz) {
                int bilet_wyslany = 0;
                
                if (czy_vip) {
                    log_print(KOLOR_PAS, tag, "VIP omija kolejke do autobusu! PID=%d", getpid());
                    if (wyslij_bilet(shm, id_pas, wiek, czy_rower, czy_vip, ma_bilet,
                                     pid_dziecka, id_dziecka, wiek_dziecka) == 0) {
                        bilet_wyslany = 1;
                    }
                } else {
                    struct sembuf wejdz = {sem_drzwi, -1, IPC_NOWAIT | SEM_UNDO};
                    struct sembuf wyjdz = {sem_drzwi, 1, SEM_UNDO};
                    if (semop(sem_id, &wejdz, 1) == 0) {
                        if (shm->aktualny_bus_pid > 0) {
                            if (wyslij_bilet(shm, id_pas, wiek, czy_rower, czy_vip, ma_bilet,
                                             pid_dziecka, id_dziecka, wiek_dziecka) == 0) {
                                bilet_wyslany = 1;
                            }
                        }
                        semop(sem_id, &wyjdz, 1);
                    }
                }
                
                if (bilet_wyslany) {
                    czekam_na_odpowiedz = 1;
                    bus_do_ktorego_wyslalem = aktualny_bus;
                }
            }
            
            // Jeśli czekamy na odpowiedź - sprawdź czy przyszła
            if (czekam_na_odpowiedz) {
                OdpowiedzMsg odp;
                if (msgrcv(msg_odp_id, &odp, sizeof(OdpowiedzMsg) - sizeof(long), 
                           getpid(), IPC_NOWAIT) != -1) {
                    czekam_na_odpowiedz = 0;
                    bus_do_ktorego_wyslalem = 0;
                    
                    if (odp.przyjety == 1) {
                        // Wsiadł do autobusu!
                        semop(sem_id, &shm_lock, 1);
                        shm->pasazerow_czeka -= ile_osob;
                        semop(sem_id, &shm_unlock, 1);
                        return 0;
                    } else {
                        // Odmowa - zapamiętaj tego busa i czekaj na następny
                        bus_ktory_odmowil = aktualny_bus;
                        log_print(KOLOR_PAS, tag, "Brak miejsc - czekam na nastepny autobus. PID=%d", getpid());
                    }
                }
            }
            
            usleep(100000);  // 100ms między próbami
            
            if (!shm->symulacja_aktywna || !shm->stacja_otwarta) {
                break;
            }
        } else {
            // Autobus odjechał - reset flag
            bus_ktory_odmowil = 0;
            czekam_na_odpowiedz = 0;
            bus_do_ktorego_wyslalem = 0;
            usleep(200000);
        }
    }

    semop(sem_id, &shm_lock, 1);
    shm->pasazerow_czeka -= ile_osob;
    semop(sem_id, &shm_unlock, 1);
    return -1;
}

static int kup_bilet(SharedData *shm, const char *tag, int id_pas, int wiek, int czy_vip, int ile_biletow) {
    struct sembuf shm_lock = {SEM_SHM, -1, SEM_UNDO};
    struct sembuf shm_unlock = {SEM_SHM, 1, SEM_UNDO};
    
    if (!czy_vip) {
        int K = shm->param_K;
        int numer_kasy = losuj(1, K);
        
        log_print(KOLOR_PAS, tag, "Kolejka do KASA %d. PID=%d", numer_kasy, getpid());
        
        KasaRequest req;
        req.mtype = numer_kasy;
        req.pid_pasazera = getpid();
        req.id_pasazera = id_pas;
        req.wiek = wiek;
        req.ile_biletow = ile_biletow;
        
        if (msgsnd(msg_kasa_id, &req, sizeof(KasaRequest) - sizeof(long), 0) == -1) {
            perror("pasazer msgsnd kasa");
            return 0;
        }
        
        KasaResponse resp;
        if (msgrcv(msg_kasa_id, &resp, sizeof(KasaResponse) - sizeof(long), getpid(), 0) == -1) {
            perror("pasazer msgrcv kasa");
            return 0;
        }
        
        log_print(KOLOR_PAS, tag, "Kupil %d bilet(y) w KASA %d. PID=%d", 
                  ile_biletow, resp.numer_kasy, getpid());
    } else {
        log_print(KOLOR_PAS, tag, "VIP - omija kolejke do kasy! PID=%d", getpid());
        semop(sem_id, &shm_lock, 1);
        if (shm->registered_count < MAX_REGISTERED) {
            shm->registered_pids[shm->registered_count] = getpid();
            shm->registered_wiek[shm->registered_count] = wiek;
            shm->registered_count++;
        }
        semop(sem_id, &shm_unlock, 1);
        log_print(KOLOR_KASA, "KASA", "VIP PAS %d (wiek=%d) - bilet okresowy", id_pas, wiek);
    }
    return 1;
}

// Zwykly dorosly pasazer (bez dziecka)
void proces_pasazer(int id_pas) {
    srand(time(NULL) ^ getpid());

    char tag[16];
    snprintf(tag, sizeof(tag), "PAS %d", id_pas);

    SharedData *shm = (SharedData *)shmat(shm_id, NULL, 0);
    if (shm == (void *)-1) { perror("pasazer shmat"); exit(1); }

    if (!shm->stacja_otwarta) {
        log_print(KOLOR_PAS, tag, "Dworzec zamkniety - nie wchodze. PID=%d", getpid());
        shmdt(shm);
        exit(0);
    }

    int wiek = losuj(9, 80);
    int czy_vip = (losuj(1, 100) == 1);
    int czy_rower = (losuj(1, 100) <= 25);

    struct sembuf shm_lock = {SEM_SHM, -1, SEM_UNDO};
    struct sembuf shm_unlock = {SEM_SHM, 1, SEM_UNDO};
    semop(sem_id, &shm_lock, 1);
    shm->total_pasazerow++;
    shm->pasazerow_czeka++;
    if (czy_vip) shm->vip_count++;
    semop(sem_id, &shm_unlock, 1);

    if (czy_rower) {
        log_print(KOLOR_PAS, tag, "Wszedl na dworzec (wiek=%d, rower)%s. PID=%d", 
                  wiek, czy_vip ? " VIP" : "", getpid());
    } else {
        log_print(KOLOR_PAS, tag, "Wszedl na dworzec (wiek=%d)%s. PID=%d", 
                  wiek, czy_vip ? " VIP" : "", getpid());
    }

    int ma_bilet = kup_bilet(shm, tag, id_pas, wiek, czy_vip, 1);

    czekaj_na_autobus(shm, tag, id_pas, wiek, czy_rower, czy_vip, ma_bilet, 0, 0, 0, 1);

    shmdt(shm);
    exit(0);
}

// Dziecko <8 lat - czeka PRZED dworcem na opiekuna
void proces_dziecko(int id_pas) {
    srand(time(NULL) ^ getpid());

    char tag[16];
    snprintf(tag, sizeof(tag), "PAS %d", id_pas);

    SharedData *shm = (SharedData *)shmat(shm_id, NULL, 0);
    if (shm == (void *)-1) { perror("dziecko shmat"); exit(1); }

    if (!shm->stacja_otwarta) {
        log_print(KOLOR_PAS, tag, "Dworzec zamkniety - dziecko odchodzi. PID=%d", getpid());
        shmdt(shm);
        exit(0);
    }

    int wiek = losuj(1, 7);
    
    struct sembuf shm_lock = {SEM_SHM, -1, SEM_UNDO};
    struct sembuf shm_unlock = {SEM_SHM, 1, SEM_UNDO};

    int moj_idx = -1;
    semop(sem_id, &shm_lock, 1);
    shm->total_pasazerow++;
    if (shm->dzieci_count < MAX_CZEKAJACE_DZIECI) {
        moj_idx = shm->dzieci_count;
        shm->dzieci[moj_idx].pid_dziecka = getpid();
        shm->dzieci[moj_idx].id_dziecka = id_pas;
        shm->dzieci[moj_idx].wiek_dziecka = wiek;
        shm->dzieci[moj_idx].ma_rodzica = false;
        shm->dzieci[moj_idx].pid_rodzica = 0;
        shm->dzieci_count++;
    }
    semop(sem_id, &shm_unlock, 1);

    log_print(KOLOR_PAS, tag, "Dziecko (wiek=%d) czeka PRZED dworcem na opiekuna. PID=%d", 
              wiek, getpid());

    if (moj_idx < 0) {
        log_print(KOLOR_PAS, tag, "Brak miejsca - dziecko odchodzi. PID=%d", getpid());
        shmdt(shm);
        exit(0);
    }

    // Czekaj na rodzica
    while (shm->symulacja_aktywna && shm->stacja_otwarta) {
        semop(sem_id, &shm_lock, 1);
        bool ma_rodzica = shm->dzieci[moj_idx].ma_rodzica;
        semop(sem_id, &shm_unlock, 1);
        
        if (ma_rodzica) {
            // Proces dziecka kończy się - dziecko kontynuuje jako WĄTEK w procesie rodzica
            shmdt(shm);
            exit(0);
        }
        usleep(200000);
    }

    // Stacja zamknieta
    semop(sem_id, &shm_lock, 1);
    shm->dzieci[moj_idx].pid_dziecka = 0;
    semop(sem_id, &shm_unlock, 1);

    log_print(KOLOR_PAS, tag, "Dziecko: dworzec zamkniety - odchodze. PID=%d", getpid());

    shmdt(shm);
    exit(0);
}

// Rodzic/opiekun który zabiera dziecko (dziecko jako wątek)
// Mechanizmy pthread:
// - pthread_create() - tworzy wątek reprezentujący dziecko
// - pthread_mutex_lock/unlock() - synchronizacja dostępu do flagi zakończenia
// - pthread_cond_wait() - wątek dziecka czeka na sygnał
// - pthread_cond_signal() - rodzic budzi wątek dziecka
// - pthread_join() - rodzic czeka na zakończenie wątku
// Rodzic/opiekun który zabiera dziecko (dziecko jako wątek)
void proces_rodzic(int id_pas, int idx_dziecka) {
    srand(time(NULL) ^ getpid());

    char tag[16];
    snprintf(tag, sizeof(tag), "PAS %d", id_pas);

    SharedData *shm = (SharedData *)shmat(shm_id, NULL, 0);
    if (shm == (void *)-1) { perror("rodzic shmat"); exit(1); }

    if (!shm->stacja_otwarta) {
        log_print(KOLOR_PAS, tag, "Dworzec zamkniety - nie wchodze. PID=%d", getpid());
        shmdt(shm);
        exit(0);
    }

    int wiek = losuj(18, 80);
    int czy_vip = (losuj(1, 100) == 1);

    struct sembuf shm_lock = {SEM_SHM, -1, SEM_UNDO};
    struct sembuf shm_unlock = {SEM_SHM, 1, SEM_UNDO};

    // Pobierz dane dziecka
    semop(sem_id, &shm_lock, 1);
    shm->total_pasazerow++;
    shm->pasazerow_czeka += 2;
    if (czy_vip) shm->vip_count++;
    
    pid_t pid_dziecka = shm->dzieci[idx_dziecka].pid_dziecka;
    int id_dziecka = shm->dzieci[idx_dziecka].id_dziecka;
    int wiek_dziecka = shm->dzieci[idx_dziecka].wiek_dziecka;
    shm->dzieci[idx_dziecka].ma_rodzica = true;
    shm->dzieci[idx_dziecka].pid_rodzica = getpid();
    semop(sem_id, &shm_unlock, 1);

    log_print(KOLOR_PAS, tag, "Opiekun (wiek=%d)%s + dziecko PAS %d (%d lat) - wchodza. PID=%d",
              wiek, czy_vip ? " VIP" : "", id_dziecka, wiek_dziecka, getpid());

    // === DZIECKO JAKO WĄTEK ===
    pthread_t tid_dziecko = 0;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
    volatile int zakoncz = 0;
    int watek_utworzony = 0;
    
    DzieckoWatekData dane_dziecka = {
        .id_dziecka = id_dziecka,
        .wiek_dziecka = wiek_dziecka,
        .mutex = &mutex,
        .cond = &cond,
        .zakoncz = &zakoncz
    };
    
    if (pthread_create(&tid_dziecko, NULL, watek_dziecko, &dane_dziecka) != 0) {
        perror("pthread_create dziecko");
        log_print(KOLOR_PAS, tag, "BLAD: Nie udalo sie utworzyc watku dziecka. PID=%d", getpid());
    } else {
        log_print(KOLOR_PAS, tag, "Dziecko PAS %d jako watek. TID=%lu, PID=%d", 
                  id_dziecka, (unsigned long)tid_dziecko, getpid());
        watek_utworzony = 1;
    }

    // Kupno biletów
    int ma_bilet = kup_bilet(shm, tag, id_pas, wiek, czy_vip, 2);

    // Rejestracja dziecka
    semop(sem_id, &shm_lock, 1);
    if (shm->registered_count < MAX_REGISTERED) {
        shm->registered_pids[shm->registered_count] = pid_dziecka;
        shm->registered_wiek[shm->registered_count] = wiek_dziecka;
        shm->registered_count++;
    }
    semop(sem_id, &shm_unlock, 1);

    // Czekanie na autobus (z potwierdzeniem)
    int wynik = czekaj_na_autobus(shm, tag, id_pas, wiek, 0, czy_vip, ma_bilet, 
                                   pid_dziecka, id_dziecka, wiek_dziecka, 2);

    // Informacja tylko gdy NIE wsiedli
    if (wynik != 0) {
        log_print(KOLOR_PAS, tag, "Opiekun + dziecko PAS %d opuszczaja dworzec. PID=%d", 
                  id_dziecka, getpid());
    }

    // Zakończ wątek dziecka
    if (watek_utworzony) {
        pthread_mutex_lock(&mutex);
        zakoncz = 1;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
        pthread_join(tid_dziecko, NULL);
    }
    
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    shmdt(shm);
    exit(0);
}

// Generator pasazerow - tworzy nowych pasazerow co 800-2000ms
void proces_generator(void) {
    srand(time(NULL) ^ getpid()); // inicjalizacja losowania
    int id_pas = 0; // licznik pasazerow

    while (1) {
        SharedData *s = (SharedData *)shmat(shm_id, NULL, 0); // polaczenie z pamiecia dzielona
        if (s == (void *)-1) exit(1);

        bool aktywna = s->symulacja_aktywna; // czy symulacja trwa
        bool otwarta = s->stacja_otwarta; // czy dworzec otwarty
        int dzieci_czekajacych = 0;
        int pierwszy_wolny_idx = -1;
        
        // policz dzieci czekajace na opiekuna
        for (int i = 0; i < s->dzieci_count; i++) {
            if (s->dzieci[i].pid_dziecka > 0 && !s->dzieci[i].ma_rodzica) {
                dzieci_czekajacych++;
                if (pierwszy_wolny_idx < 0) pierwszy_wolny_idx = i;
            }
        }
        shmdt(s); // odlacz pamiec

        if (!aktywna) break; // koniec symulacji
        if (!otwarta) { // dworzec zamkniety
            msleep(500);
            continue;
        }

        int los = losuj(1, 100); // losuj typ pasazera
        char arg_id[16];
        char arg_idx[16];
        snprintf(arg_id, sizeof(arg_id), "%d", id_pas);
        
        pid_t pas = fork(); // utworz proces potomny
        if (pas == -1) {
            perror("fork pasazer");
            continue;  // Spróbuj ponownie w następnej iteracji
        }
        if (pas == 0) { // proces dziecka

            if (los <= 15) { // 15% - dziecko
                execl("./bin/pasazer", "pasazer", "dziecko", arg_id, NULL);
            } else if (los <= 55 && dzieci_czekajacych > 0 && pierwszy_wolny_idx >= 0) { // 40% - rodzic
                snprintf(arg_idx, sizeof(arg_idx), "%d", pierwszy_wolny_idx);
                execl("./bin/pasazer", "pasazer", "rodzic", arg_id, arg_idx, NULL);
            } else { // reszta - zwykly pasazer
                execl("./bin/pasazer", "pasazer", "normal", arg_id, NULL);
            }
            exit(1);
        }
        id_pas++; // zwieksz licznik

        msleep(losuj(800, 2000)); // czekaj 800-2000ms przed nastepnym
    }
    exit(0);
}
int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uzycie: %s <typ> <id> [idx_dziecka]\n", argv[0]);
        fprintf(stderr, "  typ: normal, dziecko, rodzic, generator\n");
        exit(1);
    }
    
    if (init_ipc_client() == -1) exit(1); // polacz z IPC
    
    const char* typ = argv[1]; // pierwszy argument = typ
    
    if (strcmp(typ, "generator") == 0) { // tryb generatora
        proces_generator();
    } else if (strcmp(typ, "normal") == 0) { // zwykly pasazer
        if (argc < 3) { fprintf(stderr, "Brak id\n"); exit(1); }
        proces_pasazer(atoi(argv[2]));
    } else if (strcmp(typ, "dziecko") == 0) { // dziecko <8 lat
        if (argc < 3) { fprintf(stderr, "Brak id\n"); exit(1); }
        proces_dziecko(atoi(argv[2]));
    } else if (strcmp(typ, "rodzic") == 0) { // opiekun z dzieckiem
        if (argc < 4) { fprintf(stderr, "Rodzic wymaga id i idx_dziecka\n"); exit(1); }
        proces_rodzic(atoi(argv[2]), atoi(argv[3]));
    } else {
        fprintf(stderr, "Nieznany typ: %s\n", typ);
        exit(1);
    }
    
    return 0;
}