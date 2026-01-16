//obsluga autobusu w symulacji
//autobus jedzie na dworzec, stoi na peronie, zabiera pasazerow i jedzie do miejsca docelowego
//obsluguje sygnaly SIGUSR1 (wymuszony odjazd) i SIGTERM (koniec pracy)
#include "bus.h"
#include "common.h"

//flagi ustawiane przez handlery sygnalow
static volatile sig_atomic_t wymuszony_odjazd = 0;
static volatile sig_atomic_t bus_running = 1;
//handler sygnalow - ustawia odpowiednie flagi
static void handler_bus(int sig) {
    if (sig == SIGUSR1) {
        wymuszony_odjazd = 1;
    } else if (sig == SIGINT || sig == SIGTERM) {
        bus_running = 0;
    }
}
//sprawdza warunki zakonczenia pracy autobusu
static int czy_zakonczyc(SharedData *shm) {
    if (!shm->symulacja_aktywna) return 1;
    if (!shm->stacja_otwarta && shm->pasazerow_w_trasie <= 0) return 1;
    return 0;
}
//glowna funkcja procesu autobusu
void proces_autobus(int bus_id, int pojemnosc, int rowery, int czas_postoju) {
    srand(time(NULL) ^ getpid());
    
    //losowy czas trasy dla autobusu (15-30s)
    int czas_trasy_Ti = losuj(15000, 30000);

    //konfiguracja handlerow sygnalow
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handler_bus;
    sa.sa_flags = SA_RESTART;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    char tag[16];
    snprintf(tag, sizeof(tag), "BUS %d", bus_id);

    //dolaczenie do pamieci dzielonej
    SharedData *shm = (SharedData *)shmat(shm_id, NULL, 0);
    if (shm == (void *)-1) {
        perror("bus shmat");
        exit(1);
    }
    //struktury operacji na semaforach
    struct sembuf zwolnij_peron = {SEM_BUS_STOP, 1, SEM_UNDO};
    struct sembuf zablokuj_drzwi_n = {SEM_DOOR_NORMAL, -1, SEM_UNDO};
    struct sembuf odblokuj_drzwi_n = {SEM_DOOR_NORMAL, 1, SEM_UNDO};
    struct sembuf zablokuj_drzwi_r = {SEM_DOOR_ROWER, -1, SEM_UNDO};
    struct sembuf odblokuj_drzwi_r = {SEM_DOOR_ROWER, 1, SEM_UNDO};
    struct sembuf shm_lock = {SEM_SHM, -1, SEM_UNDO}; //blokada pamieci
    struct sembuf shm_unlock = {SEM_SHM, 1, SEM_UNDO};//odblokowanie pamieci

    int kursow = 0;
    int przewiezionych = 0;
    int mam_peron = 0;  //flaga: czy autobus zajal peron

    log_print(KOLOR_BUS, tag, "Miejsca normalne: %d, Miejsca rowerowe: %d, Ti=%dms. PID=%d", 
              pojemnosc, rowery, czas_trasy_Ti, getpid());//log startu autobusu

    //glowna petla pracy autobusu
    while (bus_running) {
        if (czy_zakonczyc(shm)) {//sprawdzenie warunkow zakonczenia
            log_print(KOLOR_BUS, tag, "Warunki zakonczenia - koncze. PID=%d", getpid());
            break;
        }
        //jazda na dworzec - pierwszy kurs krotszy (autobus startuje blizej)
        int czas_dojazdu = (kursow == 0) ? losuj(1000, 2000) : losuj(8000, 15000);
        log_print(KOLOR_BUS, tag, "Wraca na dworzec (%dms). PID=%d", czas_dojazdu, getpid());
        
        //czekanie z mozliwoscia przerwania
        int pozostalo = czas_dojazdu;
        while (pozostalo > 0 && bus_running) {
            int czekaj = (pozostalo > 500) ? 500 : pozostalo;
            msleep(czekaj);
            pozostalo -= czekaj;
            if (czy_zakonczyc(shm)) break;
        }
        if (!bus_running || czy_zakonczyc(shm)) {//sprawdzenie warunkow zakonczenia
            log_print(KOLOR_BUS, tag, "Warunki zakonczenia - koncze. PID=%d", getpid());
            break;
        }
        //proba zajecia peronu (non-blocking)
        log_print(KOLOR_BUS, tag, "Czeka na peron. PID=%d", getpid());
        struct sembuf zajmij_peron = {SEM_BUS_STOP, -1, SEM_UNDO};  //blokujacy!
        //Sprawdz przed zajmowaniem
        if (!shm->stacja_otwarta || !bus_running) {
            log_print(KOLOR_BUS, tag, "Stacja zamknieta - koncze. PID=%d", getpid());
            goto koniec;
        }
        //blokujace zajecie peronu autobus przejdzie
        if (semop(sem_id, &zajmij_peron, 1) == -1) {
            if (errno == EINTR) {
                log_print(KOLOR_BUS, tag, "Stacja zamknieta - koncze. PID=%d", getpid());
            } else {
                perror("bus semop zajmij_peron");
            }
            goto koniec;
        }
        mam_peron = 1;  //zajalem peron
        
        //Sprawdz PO zajeciu czy stacja nadal otwarta
        if (!shm->stacja_otwarta || czy_zakonczyc(shm)) {
            log_print(KOLOR_BUS, tag, "Stacja zamknieta po zajeciu peronu koncze. PID=%d", getpid());
            semop(sem_id, &zwolnij_peron, 1);
            mam_peron = 0;
            goto koniec;
        }
        //autobus zajal peron - aktualizacja stanu
        kursow++;
        wymuszony_odjazd = 0;

        semop(sem_id, &shm_lock, 1);
        shm->aktualny_bus_pid = getpid();
        shm->aktualny_bus_id = bus_id;
        shm->bus_na_peronie = true;
        shm->miejsca_zajete = 0;
        shm->rowery_zajete = 0;
        semop(sem_id, &shm_unlock, 1);

        log_print(KOLOR_BUS, tag, "[Kurs #%d] Na peronie. PID=%d, Miejsca: 0/%d, Rowery: 0/%d",
                  kursow, getpid(), pojemnosc, rowery);

        //postoj i przyjmowanie pasazerow
        int czas_na_stacji = 0;//czas spedzony na stacji
        int pasazerow_w_kursie = 0;

        if (!shm->stacja_otwarta) {
            log_print(KOLOR_BUS, tag, "Stacja zamknieta - nie przyjmuje pasazerow, koncze. PID=%d", getpid());
            //Zwolnij peron i zakoncz nie jedz pustej trasy
            semop(sem_id, &shm_lock, 1);
            shm->bus_na_peronie = false;
            shm->aktualny_bus_pid = 0;
            shm->aktualny_bus_id = 0;
            semop(sem_id, &shm_unlock, 1);
            semop(sem_id, &zwolnij_peron, 1);
            mam_peron = 0;
            break;
        } else {
            //petla przyjmowania pasazerow
            while (czas_na_stacji < czas_postoju && !wymuszony_odjazd && bus_running && shm->stacja_otwarta) {
                BiletMsg bilet;
                //najpierw VIP (mtype=PID), potem zwykli (mtype=PID+1000000)
                ssize_t ret = msgrcv(msg_id, &bilet, sizeof(BiletMsg) - sizeof(long), getpid(), IPC_NOWAIT);
                if (ret == -1 && errno == ENOMSG) {
                    ret = msgrcv(msg_id, &bilet, sizeof(BiletMsg) - sizeof(long), getpid() + 1000000, IPC_NOWAIT);
                }
                if (ret != -1) {
                    //weryfikacja biletu
                    if (!bilet.ma_bilet) {
                        log_print(KOLOR_BUS, tag, "Kierowca: PAS %d BEZ BILETU - odmowa!", bilet.id_pasazera);
                        semop(sem_id, &shm_lock, 1);
                        shm->odrzuconych_bez_biletu++;
                        semop(sem_id, &shm_unlock, 1);
                        //Wyslij odmowe do pasazera
                        OdpowiedzMsg odp_bez = { .mtype = bilet.pid_pasazera, .przyjety = 0 };
                        msgsnd(msg_id, &odp_bez, sizeof(OdpowiedzMsg) - sizeof(long), 0);
                        continue;
                    }
                    log_print(KOLOR_BUS, tag, "Kierowca: PAS %d - bilet OK.", bilet.id_pasazera);
                    semop(sem_id, &shm_lock, 1);
                    
                    //obliczenie ile miejsc potrzeba
                    int ile_osob = (bilet.wiek_dziecka > 0) ? 2 : 1;
                    bool akceptuj = true;
                    
                    //sprawdzenie dostepnosci miejsc
                    //Pasazer z rowerem zajmuje miejsce normalne + miejsce rowerowe
                    if (bilet.czy_rower) {
                        //pasazer z rowerem MUSI miec miejsce normalne i rowerowe jesli ktoregos brakuje nie wsiada
                        if (shm->miejsca_zajete + ile_osob > pojemnosc) {
                            akceptuj = false;
                        } else if (shm->rowery_zajete + ile_osob > rowery) {
                            akceptuj = false;
                        }
                    } 
                    else {
                        if (shm->miejsca_zajete + ile_osob > pojemnosc) {
                            akceptuj = false;
                        }
                    }
                    if (akceptuj) {
                        //zajecie miejsc
                        if (bilet.czy_rower) {
                            //pasazer z rowerem zajmuje oba typy miejsc
                            shm->miejsca_zajete += ile_osob;
                            shm->rowery_zajete += ile_osob;
                        } else {
                            shm->miejsca_zajete += ile_osob;
                        }
                        pasazerow_w_kursie += ile_osob;
                        int miejsca = shm->miejsca_zajete;
                        int rower_zajete = shm->rowery_zajete;
                        semop(sem_id, &shm_unlock, 1);

                        //log wsiadania
                        if (bilet.wiek_dziecka > 0) {//z dzieckiem
                            log_print(KOLOR_BUS, tag,
                                      "Wsiadl PAS %d (PID=%d, wiek=%d) z dzieckiem (wiek=%d)%s. "
                                      "Miejsca: %d/%d, Rowery: %d/%d",
                                      bilet.id_pasazera, bilet.pid_pasazera, bilet.wiek,
                                      bilet.wiek_dziecka, bilet.czy_vip ? " VIP" : "",
                                      miejsca, pojemnosc, rower_zajete, rowery);
                        } else {//bez dziecka
                            log_print(KOLOR_BUS, tag,
                                      "Wsiadl PAS %d (PID=%d, wiek=%d)%s%s. "
                                      "Miejsca: %d/%d, Rowery: %d/%d",
                                      bilet.id_pasazera, bilet.pid_pasazera, bilet.wiek,
                                      bilet.czy_rower ? " +rower" : "",
                                      bilet.czy_vip ? " VIP" : "",
                                      miejsca, pojemnosc, rower_zajete, rowery);
                        }//potwierdzenie dla pasazera
                        OdpowiedzMsg odp = { .mtype = bilet.pid_pasazera, .przyjety = 1 };
                        msgsnd(msg_id, &odp, sizeof(OdpowiedzMsg) - sizeof(long), 0);

                        //autobus odjezdza natychmiast gdy tylko wszystkie miejsca NORMALNE sa zajete
                        if (miejsca >= pojemnosc) {
                            log_print(KOLOR_BUS, tag, "PELNY (miejsca: %d/%d, rowery: %d/%d) - odjazd! PID=%d", miejsca, pojemnosc, rower_zajete, rowery, getpid());
                            break;
                        }
                    } 
                    else {//brak miejsc - odmowa
                        semop(sem_id, &shm_unlock, 1);
                        if (bilet.czy_rower && shm->rowery_zajete + ile_osob > rowery) {
                            log_print(KOLOR_BUS, tag, "Odmowa PAS %d (PID=%d): brak miejsc rowerowych",
                                      bilet.id_pasazera, bilet.pid_pasazera);
                        } else {
                            log_print(KOLOR_BUS, tag, "Odmowa PAS %d (PID=%d): brak miejsc", //np rodzic z dzieckiem 9/10 miejsc zajetych a przychodza 2 osoby
                                      bilet.id_pasazera, bilet.pid_pasazera);
                        }
                        //odmowa dla pasazera
                        OdpowiedzMsg odp = { .mtype = bilet.pid_pasazera, .przyjety = 0 };
                        msgsnd(msg_id, &odp, sizeof(OdpowiedzMsg) - sizeof(long), 0);
                    }
                }
                usleep(200000);
                czas_na_stacji += 200;
            }
            if (wymuszony_odjazd) {
                log_print(KOLOR_BUS, tag, ">>> WYMUSZONY ODJAZD (SIGUSR1)! PID=%d <<<", getpid());
            }
            //SIGUSR2 w trakcie zbierania jesli brak pasazerow zakoncz bez jazdy
            if (!shm->stacja_otwarta && pasazerow_w_kursie == 0) {
                log_print(KOLOR_BUS, tag, "Stacja zamknieta, brak pasazerow - koncze. PID=%d", getpid());
                semop(sem_id, &shm_lock, 1);
                shm->bus_na_peronie = false;
                shm->aktualny_bus_pid = 0;
                shm->aktualny_bus_id = 0;
                semop(sem_id, &shm_unlock, 1);
                semop(sem_id, &zwolnij_peron, 1);
                mam_peron = 0;
                break;
            }
        }
        //sygnalizacja odjazdu pasazerowie w drzwiach zobacza ze bus odjechal i zwolnia semafor
        semop(sem_id, &shm_lock, 1);
        shm->bus_na_peronie = false;
        shm->aktualny_bus_pid = 0;
        shm->aktualny_bus_id = 0;
        semop(sem_id, &shm_unlock, 1);
        //zamkniecie drzwi - teraz pasazerowie powinni zwolnic semafory
        log_print(KOLOR_BUS, tag, "ZAMYKAM DRZWI PRZED ODJAZDEM. PID=%d", getpid());
        semop(sem_id, &zablokuj_drzwi_n, 1);
        semop(sem_id, &zablokuj_drzwi_r, 1);
        //aktualizacja stanu po odjedzie
        semop(sem_id, &shm_lock, 1);
        shm->pasazerow_w_trasie += pasazerow_w_kursie;
        przewiezionych += pasazerow_w_kursie;
        int w_trasie = shm->pasazerow_w_trasie;
        semop(sem_id, &shm_unlock, 1);

        log_print(KOLOR_BUS, tag, "ODJAZD! Zabral %d osob. W trasie: %d. PID=%d",
                  pasazerow_w_kursie, w_trasie, getpid());
        //zwolnienie peronu i otwarcie drzwi
        semop(sem_id, &odblokuj_drzwi_n, 1);
        semop(sem_id, &odblokuj_drzwi_r, 1);
        semop(sem_id, &zwolnij_peron, 1);
        mam_peron = 0;
        //jazda do miejsca docelowego
        log_print(KOLOR_BUS, tag, "Jedzie do miejsca docelowego (%dms). PID=%d", czas_trasy_Ti, getpid());
        
        int pozostalo_trasa = czas_trasy_Ti;
        while (pozostalo_trasa > 0 && bus_running) {
            int czekaj = (pozostalo_trasa > 500) ? 500 : pozostalo_trasa;
            msleep(czekaj);
            pozostalo_trasa -= czekaj;
        }
        //pasazerowie wysiadaja w miejscu docelowym
        semop(sem_id, &shm_lock, 1);
        shm->pasazerow_w_trasie -= pasazerow_w_kursie;
        shm->total_przewiezionych += pasazerow_w_kursie;
        semop(sem_id, &shm_unlock, 1);

        log_print(KOLOR_BUS, tag, "Miejsce docelowe - pasazerowie wysiedli (%d). PID=%d", pasazerow_w_kursie, getpid());
        //koniec jesli stacja zamknieta autobus konczy po rozwiezieniu pasazerow
        if (!shm->stacja_otwarta) {
            log_print(KOLOR_BUS, tag, "Stacja zamknieta - koncze prace po rozwiezieniu pasazerow. PID=%d", getpid());
            break;
        }
    }
koniec://koniec pracy autobusu
    //Zwolnij peron jesli autobus go trzyma
    if (mam_peron) {
        semop(sem_id, &shm_lock, 1);
        shm->bus_na_peronie = false;
        shm->aktualny_bus_pid = 0;
        shm->aktualny_bus_id = 0;
        semop(sem_id, &shm_unlock, 1);
        semop(sem_id, &zwolnij_peron, 1);
        mam_peron = 0;
    }
    log_print(KOLOR_BUS, tag, "KONIEC. Kursow: %d, Przewiezionych: %d. PID=%d",
              kursow, przewiezionych, getpid());

    shmdt(shm);//odlaczenie pamieci dzielonej
    exit(0);
}
int main(int argc, char *argv[]) {
    if (argc < 5) {
        fprintf(stderr, "Uzycie: %s <bus_id> <pojemnosc> <rowery> <czas_postoju>\n", argv[0]);
        exit(1);
    }
    if (init_ipc_client() == -1) exit(1);//inicjalizacja IPC dla procesu potomnego
    proces_autobus(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));//start procesu autobusu
    return 0;
}
