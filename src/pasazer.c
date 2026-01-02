// // pasazer.c - Modul pasazera, Projekt SO 2025/2026 - Temat 12
// #include "common.h"
// #include "pasazer.h"

// void proces_pasazer(int id_pas) {
//     srand(time(NULL) ^ getpid());

//     char tag[16];
//     snprintf(tag, sizeof(tag), "PAS %d", id_pas);

//         // Podlaczenie do pamieci
//     SharedData *shm = (SharedData *)shmat(shm_id, NULL, 0);
//     if (shm == (void *)-1) {
//         perror("pasazer shmat");
//         exit(1);
//     }

//         // Sprawdz czy stacja otwarta
//     if (!shm->stacja_otwarta) {
//         log_print(KOLOR_PAS, tag, "Dworzec zamknięty - nie wchodzę. PID=%d", getpid());
//         shmdt(shm);
//         exit(0);
//     }

//         // GENEROWANIE ATRUBYTOW
//     int wiek = losuj(1, 80);
//     int czy_vip = (losuj(1, 100) == 1);     /* 1% VIP */
//     int czy_rower = (losuj(1, 100) <= 15);  /* 15% z rowerem */
//     int wiek_dziecka = 0;

//         // Dziecko <8 lat musi miec opiekuna
//     if (wiek < 8) {
//             // To jest dziecko - musi byc z rodzicem
//         wiek_dziecka = wiek;
//         wiek = losuj(25, 50);  /* Rodzic */
//         czy_rower = 0;  /* Z dzieckiem bez roweru */
//     } else if (wiek >= 25 && wiek <= 50 && losuj(1, 100) <= 20) {
//             // Dorosly moze miec dziecko (20% szans)
//         wiek_dziecka = losuj(1, 7);
//         czy_rower = 0;
//     }

//         // Aktualizacja statystyk
//     struct sembuf shm_lock = {SEM_SHM, -1, 0};
//     struct sembuf shm_unlock = {SEM_SHM, 1, 0};

//     semop(sem_id, &shm_lock, 1);
//     shm->total_pasazerow++;
//     shm->pasazerow_czeka++;
//     if (czy_vip) shm->vip_count++;
//     semop(sem_id, &shm_unlock, 1);

//         // Logowanie pojawienia sie
//     if (wiek_dziecka > 0) {
//         log_print(KOLOR_PAS, tag, 
//                   "Przybył. PID=%d, Wiek=%d, Dziecko=%d lat, VIP=%d, Rower=%d",
//                   getpid(), wiek, wiek_dziecka, czy_vip, czy_rower);
//     } else {
//         log_print(KOLOR_PAS, tag, 
//                   "Przybył. PID=%d, Wiek=%d, VIP=%d, Rower=%d",
//                   getpid(), wiek, czy_vip, czy_rower);
//     }

//         // KUPNO BILETU W KASIE
//     if (!czy_vip) {
//         /* Wybór okienka (losowo 1 lub 2) */
//         int K = shm->param_K;
//         int numer_kasy = losuj(1, K);
//         int sem_kasa = SEM_KASA_BASE + (numer_kasy - 1);
//         struct sembuf zajmij_kase = {sem_kasa, -1, 0};
//         struct sembuf zwolnij_kase = {sem_kasa, 1, 0};

//         log_print(KOLOR_PAS, tag, "Czeka w kolejce do kasy. PID=%d", getpid());

//         if (semop(sem_id, &zajmij_kase, 1) == -1) {
//             shmdt(shm);
//             exit(1);
//         }

//         /* Rejestracja w kasie */
//         semop(sem_id, &shm_lock, 1);
//         if (shm->registered_count < MAX_REGISTERED) {
//             shm->registered_pids[shm->registered_count] = getpid();
//             shm->registered_wiek[shm->registered_count] = wiek;
//             shm->registered_count++;
//         }
//         shm->sprzedanych_biletow += 1;
//         shm->obsluzonych_kasa[numer_kasy - 1]++;        semop(sem_id, &shm_unlock, 1);

//         /* Symulacja czasu zakupu */
//         msleep(losuj(200, 500));

//         semop(sem_id, &zwolnij_kase, 1);

//         log_print(KOLOR_KASA, "KASA", 
//                   "Zarejestrowano PAS %d (PID=%d, wiek=%d)", id_pas, getpid(), wiek);
//         log_print(KOLOR_PAS, tag, "Ma bilet. Czeka na autobus. PID=%d", getpid());
//     } else {
//         log_print(KOLOR_PAS, tag, "(VIP) Omija kolejkę! PID=%d", getpid());
        
//         /* VIP też rejestrowany */
//         semop(sem_id, &shm_lock, 1);
//         if (shm->registered_count < MAX_REGISTERED) {
//             shm->registered_pids[shm->registered_count] = getpid();
//             shm->registered_wiek[shm->registered_count] = wiek;
//             shm->registered_count++;
//         }
//         semop(sem_id, &shm_unlock, 1);
//     }

//         // OCZEKIWANIE NA AUTOBUS
//     int sem_drzwi = czy_rower ? SEM_DOOR_ROWER : SEM_DOOR_NORMAL;

//     // Czekaj dopoki symulacja aktywna
//     while (shm->symulacja_aktywna) {
//         // SIGUSR2 - dworzec zamkniety = nie mozna wsiasc, pasazer opuszcza dworzec
//         if (!shm->stacja_otwarta) {
//             log_print(KOLOR_PAS, tag, "Dworzec zamkniety - opuszczam dworzec. PID=%d", getpid());
//             semop(sem_id, &shm_lock, 1);
//             shm->pasazerow_czeka--;
//             semop(sem_id, &shm_unlock, 1);
//             shmdt(shm);
//             exit(0);
//         }

//         /* Sprawdź czy jest autobus */
//         if (shm->bus_na_peronie && shm->aktualny_bus_pid > 0) {
//             struct sembuf wejdz = {sem_drzwi, -1, IPC_NOWAIT};
//             struct sembuf wyjdz = {sem_drzwi, 1, 0};

//             if (semop(sem_id, &wejdz, 1) == 0) {
//                 /* W drzwiach - sprawdź czy autobus nadal jest */
//                 if (shm->aktualny_bus_pid > 0) {
//                     BiletMsg bilet;
//                     memset(&bilet, 0, sizeof(bilet));
//                     bilet.mtype = czy_vip ? shm->aktualny_bus_pid : (shm->aktualny_bus_pid + 1000000);//VIP ma inny typ priorytetowy
//                     bilet.pid_pasazera = getpid();
//                     bilet.id_pasazera = id_pas;
//                     bilet.wiek = wiek;
//                     bilet.wiek_dziecka = wiek_dziecka;
//                     bilet.czy_rower = czy_rower;
//                     bilet.czy_vip = czy_vip;
//                     bilet.ma_bilet = 1;
//                     /* Wyślij bilet */
//                     if (msgsnd(msg_id, &bilet, sizeof(BiletMsg) - sizeof(long), 0) == 0) {
//                         /* Aktualizacja */
//                         semop(sem_id, &shm_lock, 1);
//                         shm->pasazerow_czeka--;
//                         semop(sem_id, &shm_unlock, 1);

//                         /* Zwolnij drzwi */
//                         semop(sem_id, &wyjdz, 1);

//                         shmdt(shm);
//                         exit(0);  /* Sukces! */
//                     }
//                 }
//                 semop(sem_id, &wyjdz, 1);
//             }

//             /* VIP czeka krócej */
//             usleep(czy_vip ? 50000 : 100000);
//         } else {
//             usleep(200000);
//         }
//     }

//     // Symulacja zakonczona
//     semop(sem_id, &shm_lock, 1);
//     shm->pasazerow_czeka--;
//     semop(sem_id, &shm_unlock, 1);

//     shmdt(shm);
//     exit(0);
// }
// int main(int argc, char *argv[]) {
//     if (argc < 3) {
//         fprintf(stderr, "Uzycie: %s <typ> <id>\n", argv[0]);
//         exit(1);
//     }
    
//     if (init_ipc_client() == -1) exit(1);
    
//     proces_pasazer(atoi(argv[2]));
//     return 0;
// }

// pasazer.c - Modul pasazera
#include "common.h"
#include "pasazer.h"

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
    struct sembuf shm_lock = {SEM_SHM, -1, 0};
    struct sembuf shm_unlock = {SEM_SHM, 1, 0};
    int sem_drzwi = czy_rower ? SEM_DOOR_ROWER : SEM_DOOR_NORMAL;

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
            if (czy_vip) {
                log_print(KOLOR_PAS, tag, "VIP omija kolejke do autobusu! PID=%d", getpid());
                if (wyslij_bilet(shm, id_pas, wiek, czy_rower, czy_vip, ma_bilet,
                                 pid_dziecka, id_dziecka, wiek_dziecka) == 0) {
                    semop(sem_id, &shm_lock, 1);
                    shm->pasazerow_czeka -= ile_osob;
                    semop(sem_id, &shm_unlock, 1);
                    return 0;
                }
                usleep(30000);
            } else {
                // struct sembuf wejdz = {sem_drzwi, -1, IPC_NOWAIT};
                // struct sembuf wyjdz = {sem_drzwi, 1, 0};
                struct sembuf wejdz = {sem_drzwi, -1, IPC_NOWAIT | SEM_UNDO};
                struct sembuf wyjdz = {sem_drzwi, 1, SEM_UNDO};
                if (semop(sem_id, &wejdz, 1) == 0) {
                    if (shm->aktualny_bus_pid > 0) {
                        if (wyslij_bilet(shm, id_pas, wiek, czy_rower, czy_vip, ma_bilet,
                                         pid_dziecka, id_dziecka, wiek_dziecka) == 0) {
                            semop(sem_id, &shm_lock, 1);
                            shm->pasazerow_czeka -= ile_osob;
                            semop(sem_id, &shm_unlock, 1);
                            semop(sem_id, &wyjdz, 1);
                            return 0;
                        }
                    }
                    semop(sem_id, &wyjdz, 1);
                }
                usleep(100000);
            }
        } else {
            usleep(200000);
        }
    }

    semop(sem_id, &shm_lock, 1);
    shm->pasazerow_czeka -= ile_osob;
    semop(sem_id, &shm_unlock, 1);
    return -1;
}

// Obsluga kupowania biletu w kasie
static int kup_bilet(SharedData *shm, const char *tag, int id_pas, int wiek, int czy_vip, int ile_biletow) {
    struct sembuf shm_lock = {SEM_SHM, -1, 0};
    struct sembuf shm_unlock = {SEM_SHM, 1, 0};

    if (!czy_vip) {
        int K = shm->param_K;
        int numer_kasy = losuj(1, K);
        int sem_kasa = SEM_KASA_BASE + (numer_kasy - 1);
        // struct sembuf zajmij = {sem_kasa, -1, 0};
        // struct sembuf zwolnij = {sem_kasa, 1, 0};

        struct sembuf shm_lock = {SEM_SHM, -1, SEM_UNDO};
        struct sembuf shm_unlock = {SEM_SHM, 1, SEM_UNDO};

        char tag_kasa[16];
        snprintf(tag_kasa, sizeof(tag_kasa), "KASA %d", numer_kasy);

        log_print(KOLOR_PAS, tag, "Kolejka do KASA %d. PID=%d", numer_kasy, getpid());
        
        if (semop(sem_id, &zajmij, 1) == -1) {
            perror("pasazer semop kasa");
            return 0;
        }

        semop(sem_id, &shm_lock, 1);
        if (shm->registered_count < MAX_REGISTERED) {
            shm->registered_pids[shm->registered_count] = getpid();
            shm->registered_wiek[shm->registered_count] = wiek;
            shm->registered_count++;
        }
        shm->sprzedanych_biletow += ile_biletow;
        shm->obsluzonych_kasa[numer_kasy - 1]++;
        semop(sem_id, &shm_unlock, 1);

        msleep(losuj(200, 500));
        semop(sem_id, &zwolnij, 1);

        log_print(KOLOR_KASA, tag_kasa, "Sprzedano %d bilet(y) PAS %d (wiek=%d)", 
                  ile_biletow, id_pas, wiek);
        log_print(KOLOR_PAS, tag, "Kupil %d bilet(y). PID=%d", ile_biletow, getpid());
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

    int wiek = losuj(18, 80);
    int czy_vip = (losuj(1, 100) == 1);
    int czy_rower = (losuj(1, 100) <= 15);

    struct sembuf shm_lock = {SEM_SHM, -1, 0};
    struct sembuf shm_unlock = {SEM_SHM, 1, 0};

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
    struct sembuf shm_lock = {SEM_SHM, -1, 0};
    struct sembuf shm_unlock = {SEM_SHM, 1, 0};

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
            log_print(KOLOR_PAS, tag, "Dziecko: opiekun przyszedl - wchodzimy! PID=%d", getpid());
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

// Rodzic/opiekun ktory zabiera dziecko
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

    int wiek = losuj(25, 50);
    int czy_vip = (losuj(1, 100) == 1);

    // struct sembuf shm_lock = {SEM_SHM, -1, 0};
    // struct sembuf shm_unlock = {SEM_SHM, 1, 0};

    struct sembuf shm_lock = {SEM_SHM, -1, SEM_UNDO};
    struct sembuf shm_unlock = {SEM_SHM, 1, SEM_UNDO};

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

    int ma_bilet = kup_bilet(shm, tag, id_pas, wiek, czy_vip, 2);

    semop(sem_id, &shm_lock, 1);
    if (shm->registered_count < MAX_REGISTERED) {
        shm->registered_pids[shm->registered_count] = pid_dziecka;
        shm->registered_wiek[shm->registered_count] = wiek_dziecka;
        shm->registered_count++;
    }
    semop(sem_id, &shm_unlock, 1);

    czekaj_na_autobus(shm, tag, id_pas, wiek, 0, czy_vip, ma_bilet, 
                      pid_dziecka, id_dziecka, wiek_dziecka, 2);

    shmdt(shm);
    exit(0);
}

// Main - uruchamiany przez exec() z dyspozytora
int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Uzycie: %s <typ> <id> [idx_dziecka]\n", argv[0]);
        fprintf(stderr, "  typ: normal, dziecko, rodzic\n");
        exit(1);
    }
    
    if (init_ipc_client() == -1) exit(1);
    
    const char* typ = argv[1];
    int id = atoi(argv[2]);
    
    if (strcmp(typ, "normal") == 0) {
        proces_pasazer(id);
    } else if (strcmp(typ, "dziecko") == 0) {
        proces_dziecko(id);
    } else if (strcmp(typ, "rodzic") == 0) {
        if (argc < 4) {
            fprintf(stderr, "Rodzic wymaga idx_dziecka\n");
            exit(1);
        }
        proces_rodzic(id, atoi(argv[3]));
    } else {
        fprintf(stderr, "Nieznany typ: %s\n", typ);
        exit(1);
    }
    
    return 0;
}