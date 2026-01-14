//obsluga pasazerow w symulacji
//generator tworzy nowych pasazerow co 800-2000ms
//normal-dorosly 9-80 lat, kupuje 1 bilet
//rodzic_z_dzieckiem- rodzic 18-80 + dziecko 1-7 jako 2 watki w jednym procesie
//VIP (1% szans) omija kolejke do kasy i ma priorytet w autobusie
//ROWER zajmuje miejsce w puli rowerowej autobusu
#include "common.h"
#include "pasazer.h"

//funkcja watku dziecka czeka na sygnał zakończenia od rodzica
static void* watek_dziecko(void* arg) {
    DzieckoWatekData *d = (DzieckoWatekData *)arg;
    
    char tag[16];
    snprintf(tag, sizeof(tag), "PAS %d", d->id_dziecka);
    
    log_print(KOLOR_PAS, tag, "[WATEK] Dziecko (wiek=%d) z rodzicem TID=%lu", 
              d->wiek_dziecka, (unsigned long)pthread_self());
    pthread_mutex_lock(d->mutex);//Czekaj na sygnał zakończenia od rodzica

    while (!(*d->zakoncz)) {
        pthread_cond_wait(d->cond, d->mutex);
    }
    pthread_mutex_unlock(d->mutex);
    
    log_print(KOLOR_PAS, tag, "[WATEK] Dziecko konczy dzialanie TID=%lu", 
              (unsigned long)pthread_self());
    
    return NULL;
}
//wysylanie biletu do autobusu VIP ma mtype=PID autobusu, zwykly ma mtype=PID+1000000 (nizszy priorytet)
static int wyslij_bilet(SharedData *shm, int id_pas, int wiek, int czy_rower, int czy_vip,
                        int ma_bilet, int id_dziecka, int wiek_dziecka) {
    BiletMsg bilet;
    memset(&bilet, 0, sizeof(bilet));
    
    bilet.mtype = czy_vip ? shm->aktualny_bus_pid : (shm->aktualny_bus_pid + 1000000);
    bilet.pid_pasazera = getpid();
    bilet.id_pasazera = id_pas;
    bilet.wiek = wiek;
    bilet.czy_rower = czy_rower;
    bilet.czy_vip = czy_vip;
    bilet.ma_bilet = ma_bilet;
    bilet.id_dziecka = id_dziecka;
    bilet.wiek_dziecka = wiek_dziecka;

    return msgsnd(msg_id, &bilet, sizeof(BiletMsg) - sizeof(long), 0);
}
//glowna petla oczekiwania na autobus - obsluguje flagi: bus_ktory_odmowil, czekam_na_odpowiedz, juz_wsiadlem
static int czekaj_na_autobus(SharedData *shm, const char *tag, int id_pas, int wiek, 
                              int czy_rower, int czy_vip, int ma_bilet,
                              int id_dziecka, int wiek_dziecka,
                              int ile_osob) {

    struct sembuf shm_lock = {SEM_SHM, -1, SEM_UNDO};
    struct sembuf shm_unlock = {SEM_SHM, 1, SEM_UNDO};  
    
    //flagi stanu pasazera
    pid_t bus_ktory_odmowil = 0;
    int czekam_na_odpowiedz = 0;
    pid_t bus_do_ktorego_wyslalem = 0;
    int juz_wsiadlem = 0;

    while (shm->symulacja_aktywna) {
        if (juz_wsiadlem) {
            return 0;
        }
        //obsluga zamkniecia dworca
        if (!shm->stacja_otwarta) {
            log_print(KOLOR_PAS, tag, "Dworzec zamkniety - opuszczam PID=%d", getpid());
            semop(sem_id, &shm_lock, 1);
            shm->pasazerow_czeka -= ile_osob;
            semop(sem_id, &shm_unlock, 1);
            return -1;
        }
        if (shm->bus_na_peronie && shm->aktualny_bus_pid > 0) {
            pid_t aktualny_bus = shm->aktualny_bus_pid;
            
            //ten autobus juz odmowil czekaj na inny
            if (aktualny_bus == bus_ktory_odmowil) {
                usleep(200000);  
                continue;
            }
            //autobus sie zmienil reset stanu
            if (czekam_na_odpowiedz && aktualny_bus != bus_do_ktorego_wyslalem) {
                czekam_na_odpowiedz = 0;
                bus_do_ktorego_wyslalem = 0;
            }
            //wysylanie biletu jesli jeszcze nie wyslano
            if (!czekam_na_odpowiedz) {
                int bilet_wyslany = 0;
                if (czy_vip) {
                    log_print(KOLOR_PAS, tag, "VIP omija kolejke do autobusu! PID=%d", getpid());
                    if (wyslij_bilet(shm, id_pas, wiek, czy_rower, czy_vip, ma_bilet,
                                     id_dziecka, wiek_dziecka) == 0) {
                        bilet_wyslany = 1;
                    }
                } else {
                    //Pasazer z rowerem musi przejsc przez OBA semafory (rower + normalny)
                    //Pasazer bez roweru tylko przez normalny
                    if (czy_rower) {
                        struct sembuf wejdz_r = {SEM_DOOR_ROWER, -1, IPC_NOWAIT | SEM_UNDO};
                        struct sembuf wyjdz_r = {SEM_DOOR_ROWER, 1, SEM_UNDO};
                        struct sembuf wejdz_n = {SEM_DOOR_NORMAL, -1, IPC_NOWAIT | SEM_UNDO};
                        struct sembuf wyjdz_n = {SEM_DOOR_NORMAL, 1, SEM_UNDO};
                        
                        //najpierw drzwi rowerowe
                        if (semop(sem_id, &wejdz_r, 1) == 0) {
                            //potem drzwi normalne
                            if (semop(sem_id, &wejdz_n, 1) == 0) {
                                if (shm->aktualny_bus_pid > 0) {
                                    if (wyslij_bilet(shm, id_pas, wiek, czy_rower, czy_vip, ma_bilet,
                                                     id_dziecka, wiek_dziecka) == 0) {
                                        bilet_wyslany = 1;
                                    }
                                }
                                semop(sem_id, &wyjdz_n, 1);  //zwolnij normalne
                            }
                            semop(sem_id, &wyjdz_r, 1);  //zwolnij rowerowe
                        }
                    } else {
                        struct sembuf wejdz = {SEM_DOOR_NORMAL, -1, IPC_NOWAIT | SEM_UNDO};
                        struct sembuf wyjdz = {SEM_DOOR_NORMAL, 1, SEM_UNDO};
                        if (semop(sem_id, &wejdz, 1) == 0) {
                            if (shm->aktualny_bus_pid > 0) {
                                if (wyslij_bilet(shm, id_pas, wiek, czy_rower, czy_vip, ma_bilet,
                                                 id_dziecka, wiek_dziecka) == 0) {
                                    bilet_wyslany = 1;
                                }
                            }
                            semop(sem_id, &wyjdz, 1);
                        }
                    }
                }
                if (bilet_wyslany) {
                    czekam_na_odpowiedz = 1;
                    bus_do_ktorego_wyslalem = aktualny_bus;
                }
            }
            //sprawdzanie odpowiedzi od autobusu
            if (czekam_na_odpowiedz) {
                OdpowiedzMsg odp;
                if (msgrcv(msg_id, &odp, sizeof(OdpowiedzMsg) - sizeof(long), 
                           getpid(), IPC_NOWAIT) != -1) {
                    czekam_na_odpowiedz = 0;
                    bus_do_ktorego_wyslalem = 0;
                    
                    if (odp.przyjety == 1) {
                        //udalo sie wsiasc
                        juz_wsiadlem = 1;
                        semop(sem_id, &shm_lock, 1);
                        shm->pasazerow_czeka -= ile_osob;
                        semop(sem_id, &shm_unlock, 1);
                        return 0;
                    } else {
                        //odmowa - sprawdz czy to z powodu braku biletu
                        if (!ma_bilet) {
                            //bez biletu - kierowca odrzucil, opuszczam dworzec
                            log_print(KOLOR_PAS, tag, "Odrzucony przez kierowce (brak biletu) - opuszczam dworzec. PID=%d", getpid());
                            semop(sem_id, &shm_lock, 1);
                            shm->pasazerow_czeka -= ile_osob;
                            semop(sem_id, &shm_unlock, 1);
                            return -1;
                        }
                        //odmowa z powodu braku miejsc - czekaj na nastepny
                        bus_ktory_odmowil = aktualny_bus;
                        log_print(KOLOR_PAS, tag, "Brak miejsc - czekam na nastepny autobus PID=%d", getpid());
                    }
                }
            }
            usleep(100000);
            
            if (!shm->symulacja_aktywna || !shm->stacja_otwarta) {
                break;
            }
        } else {
            //brak autobusu na peronie reset flag
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
}//funkcja kupowania biletu (zwykly lub VIP)
static int kup_bilet(SharedData *shm, const char *tag, int id_pas, int wiek, int czy_vip, int ile_biletow) {
    struct sembuf shm_lock = {SEM_SHM, -1, SEM_UNDO};
    struct sembuf shm_unlock = {SEM_SHM, 1, SEM_UNDO};
    
    if (!czy_vip) {
        int numer_kasy = losuj(1, shm->param_K);
        
        log_print(KOLOR_PAS, tag, "Kolejka do KASA %d. PID=%d", numer_kasy, getpid());
        //wysylanie zapytania do kasy
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
        if (resp.sukces == 0) {
            if (resp.brak_srodkow) {
                //odmowa z powodu braku srodkow - pasazer idzie bez biletu
                log_print(KOLOR_PAS, tag, "BRAK SRODKOW - nie kupil biletu, idzie na dworzec. PID=%d", getpid());
                return 0;  //zwraca 0 = brak biletu
            }
            //stacja zamknieta
            return -1;  //zwraca -1 = opusc dworzec
        }
        log_print(KOLOR_PAS, tag, "Kupil %d bilet(y) w KASA %d. PID=%d", 
                  ile_biletow, resp.numer_kasy, getpid());
    } else {
        log_print(KOLOR_PAS, tag, "VIP - omija kase! PID=%d", getpid());//VIP nie kupuje biletu w kasie
        //rejestracja VIPa w pamieci dzielonej
        semop(sem_id, &shm_lock, 1);
        if (shm->registered_count < MAX_REGISTERED) {
            shm->registered_pids[shm->registered_count] = getpid();
            shm->registered_wiek[shm->registered_count] = wiek;
            shm->registered_count++;
        }
        semop(sem_id, &shm_unlock, 1);
        log_print(KOLOR_KASA, "KASA", "VIP PAS %d (wiek=%d) - Wczesniej wykupiony bilet", id_pas, wiek);
    }
    return 1;
}
//proces zwyklego pasazera (dorosly 9-80 lat, bez dziecka)
void proces_pasazer(int id_pas) {
    srand(time(NULL) ^ getpid());

    char tag[16];
    snprintf(tag, sizeof(tag), "PAS %d", id_pas);//tag do logow

    SharedData *shm = (SharedData *)shmat(shm_id, NULL, 0);
    if (shm == (void *)-1) { perror("pasazer shmat"); exit(1); }

    if (!shm->stacja_otwarta) {//sprawdzenie czy dworzec otwarty
        log_print(KOLOR_PAS, tag, "Dworzec zamkniety - nie wchodze PID=%d", getpid());
        shmdt(shm);
        exit(0);
    }
    int wiek = losuj(9, 80);
    int czy_vip = (losuj(1, 100) == 1);
    int czy_rower = (losuj(1, 100) <= 25);
    //aktualizacja statystyk
    struct sembuf shm_lock = {SEM_SHM, -1, SEM_UNDO};
    struct sembuf shm_unlock = {SEM_SHM, 1, SEM_UNDO};
    semop(sem_id, &shm_lock, 1);
    shm->total_pasazerow++;
    shm->pasazerow_czeka++;
    if (czy_vip) shm->vip_count++;
    semop(sem_id, &shm_unlock, 1);

    log_print(KOLOR_PAS, tag, "Wszedl na dworzec (wiek=%d%s)%s. PID=%d", 
              wiek, czy_rower ? ", rower" : "", czy_vip ? " VIP" : "", getpid());

    int ma_bilet = kup_bilet(shm, tag, id_pas, wiek, czy_vip, 1);
    
    //ma_bilet: 1=kupiony, 0=brak srodkow (idz bez biletu), -1=stacja zamknieta
    if (ma_bilet == -1) {
        log_print(KOLOR_PAS, tag, "Stacja zamknieta - opuszczam dworzec. PID=%d", getpid());
        semop(sem_id, &shm_lock, 1);
        shm->pasazerow_czeka--;
        semop(sem_id, &shm_unlock, 1);
        shmdt(shm);
        exit(0);
    }
    
    czekaj_na_autobus(shm, tag, id_pas, wiek, czy_rower, czy_vip, ma_bilet, 0, 0, 1);

    shmdt(shm);
    exit(0);
}
//proces rodzica z dzieckiem - jeden proces, dwa watki
void proces_rodzic_z_dzieckiem(int id_pas) {
    srand(time(NULL) ^ getpid());

    char tag[16];
    snprintf(tag, sizeof(tag), "PAS %d", id_pas);

    SharedData *shm = (SharedData *)shmat(shm_id, NULL, 0);
    if (shm == (void *)-1) { perror("rodzic shmat"); exit(1); }

    if (!shm->stacja_otwarta) {
        log_print(KOLOR_PAS, tag, "Dworzec zamkniety - nie wchodze PID=%d", getpid());
        shmdt(shm);
        exit(0);
    }
    //dane rodzica i dziecka
    int wiek = losuj(18, 80);
    int czy_vip = 0;//rodzic nie jest VIPem
    int id_dziecka = id_pas;
    int wiek_dziecka = losuj(1, 7);

    struct sembuf shm_lock = {SEM_SHM, -1, SEM_UNDO};
    struct sembuf shm_unlock = {SEM_SHM, 1, SEM_UNDO};
    
    //rejestracja obu osob w statystykach
    semop(sem_id, &shm_lock, 1);
    shm->total_pasazerow += 2;
    shm->pasazerow_czeka += 2;
    shm->rodzicow_z_dziecmi++;  //zliczanie rodzicow z dziecmi
    if (czy_vip) shm->vip_count++;
    semop(sem_id, &shm_unlock, 1);

    log_print(KOLOR_PAS, tag, "Opiekun (wiek=%d)%s + dziecko PAS %d (%d lat) - wchodza na dworzec. PID=%d",
              wiek, czy_vip ? " VIP" : "", id_dziecka, wiek_dziecka, getpid());

    //tworzenie watku dziecka
    pthread_t tid_dziecko = 0;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
    volatile int zakoncz = 0;
    int watek_utworzony = 0;
    //inicjalizacja danych watku dziecka
    DzieckoWatekData dane_dziecka = {
        .id_dziecka = id_dziecka,
        .wiek_dziecka = wiek_dziecka,
        .mutex = &mutex,
        .cond = &cond,
        .zakoncz = &zakoncz
    };//utworzenie watku dziecka
    if (pthread_create(&tid_dziecko, NULL, watek_dziecko, &dane_dziecka) != 0) {
        perror("pthread_create dziecko");
        log_print(KOLOR_PAS, tag, "BLAD: Nie udalo sie utworzyc watku dziecka. PID=%d", getpid());
    } else {
        log_print(KOLOR_PAS, tag, "Dziecko PAS %d jako watek. TID=%lu, PID=%d", 
                  id_dziecka, (unsigned long)tid_dziecko, getpid());
        watek_utworzony = 1;
    }

    //kupno 2 biletow i oczekiwanie na autobus
    int ma_bilet = kup_bilet(shm, tag, id_pas, wiek, czy_vip, 2);
    
    //ma_bilet: 1=kupiony, 0=brak srodkow (idz bez biletu), -1=stacja zamknieta
    if (ma_bilet == -1) {
        log_print(KOLOR_PAS, tag, "Stacja zamknieta - opiekun + dziecko opuszczaja dworzec. PID=%d", getpid());
        semop(sem_id, &shm_lock, 1);
        shm->pasazerow_czeka -= 2;
        semop(sem_id, &shm_unlock, 1);
        
        //zakonczenie watku dziecka
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
    
    int wynik = czekaj_na_autobus(shm, tag, id_pas, wiek, 0, czy_vip, ma_bilet, 
                                   id_dziecka, wiek_dziecka, 2);

    if (wynik != 0) {
        log_print(KOLOR_PAS, tag, "Opiekun + dziecko PAS %d opuszczaja dworzec. PID=%d", 
                  id_dziecka, getpid());
    }
    //zakonczenie watku dziecka
    if (watek_utworzony) {
        pthread_mutex_lock(&mutex);
        zakoncz = 1;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
        pthread_join(tid_dziecko, NULL);
    }//sprzatanie
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    shmdt(shm);
    exit(0);
}
//generator pasazerow - tworzy nowych co 800-2000ms
void proces_generator(void) {
    signal(SIGCHLD, SIG_IGN);
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
        //losowanie typu pasazera i tworzenie procesu
        int los = losuj(1, 100);
        char arg_id[16];
        snprintf(arg_id, sizeof(arg_id), "%d", id_pas);
        
        pid_t pas = fork();
        if (pas == -1) {
            perror("fork pasazer");
            continue;//sprobuj ponownie w nastepnej iteracji
        }
        if (pas == 0) {//proces dziecka
            if (los <= 20) {//20% rodzic z dzieckiem
                execl("./bin/pasazer", "pasazer", "rodzic_z_dzieckiem", arg_id, NULL);
            } else {//80% zwykly pasazer
                execl("./bin/pasazer", "pasazer", "normal", arg_id, NULL);
            }
            exit(1);
        }
        id_pas++;
        msleep(losuj(800, 2000));
    }
    exit(0);
}
int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uzycie: %s <typ> <id>\n", argv[0]);
        fprintf(stderr, "  typ: normal, rodzic_z_dzieckiem, generator\n");
        exit(1);
    }
    if (init_ipc_client() == -1) exit(1);//inicjalizacja IPC
    
    const char* typ = argv[1];
    
    if (strcmp(typ, "generator") == 0) {//proces generatora pasazerow
        proces_generator();
    } else if (strcmp(typ, "normal") == 0) {//pojedynczy proces pasazera
        if (argc < 3) { fprintf(stderr, "Brak id\n"); exit(1); }
        proces_pasazer(atoi(argv[2]));
    } else if (strcmp(typ, "rodzic_z_dzieckiem") == 0) {// jeden proces, dwa watki
        if (argc < 3) { fprintf(stderr, "Brak id\n"); exit(1); }
        proces_rodzic_z_dzieckiem(atoi(argv[2]));
    } else {
        fprintf(stderr, "Nieznany typ: %s\n", typ);
        exit(1);
    }
    return 0;
}
