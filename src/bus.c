//obsluga autobusu w symulacji
//autobus jedzie na dworzec, stoi na peronie, zabiera pasazerow i jedzie do miejsca docelowego
//obsluguje sygnaly SIGUSR1 (wymuszony odjazd) i SIGTERM (koniec pracy)
#include "bus.h"
#include "common.h"
#include <sched.h>

//flagi ustawiane przez handlery sygnalow
static volatile sig_atomic_t wymuszony_odjazd = 0;
static volatile sig_atomic_t bus_running = 1;
//handler sygnalow - ustawia odpowiednie flagi
static void handler_bus(int sig) {
    if (sig == SIGUSR1) {
        wymuszony_odjazd = 1;
    } else if (sig == SIGINT || sig == SIGTERM || sig == SIGQUIT) {
        bus_running = 0;
    }
}
//sprawdza warunki zakonczenia pracy autobusu
static int czy_zakonczyc(SharedData *shm) {
    if (!shm->symulacja_aktywna) return 1;
    if (!shm->dworzec_otwarty && shm->pasazerow_w_trasie <= 0) return 1;
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
    sigaction(SIGQUIT, &sa, NULL);

    char tag[16];
    snprintf(tag, sizeof(tag), "BUS %d", bus_id);

    //dolaczenie do pamieci dzielonej
    SharedData *shm = (SharedData *)shmat(shm_id, NULL, 0);
    if (shm == (void *)-1) {
        perror("bus shmat");
        exit(1);
    }
    //struktury operacji na semaforach
    struct sembuf zwolnij_przystanek = {SEM_BUS_STOP, 1, SEM_UNDO};
    struct sembuf odblokuj_drzwi_n = {SEM_DOOR_NORMAL, 1, SEM_UNDO};
    struct sembuf odblokuj_drzwi_r = {SEM_DOOR_ROWER, 1, SEM_UNDO};
    struct sembuf shm_lock = {SEM_SHM, -1, SEM_UNDO};  
    struct sembuf shm_unlock = {SEM_SHM, 1, SEM_UNDO};

    int kursow = 0;
    int przewiezionych = 0;
    int mam_przystanek = 0;//flaga czy autobus zajal przystanek
    time_t czas_start, czas_koniec;//do mierzenia czasu 

    log_print(KOLOR_BUS, tag, "Miejsca normalne: %d, Miejsca rowerowe: %d, Ti=%dms. PID=%d", 
              pojemnosc, rowery, czas_trasy_Ti, getpid());//log startu autobusu

    //glowna petla pracy autobusu
    while (bus_running) {
        if (czy_zakonczyc(shm)) {//sprawdzenie warunkow zakonczenia
            log_print(KOLOR_BUS, tag, "Warunki zakonczenia - koncze. PID=%d", getpid());
            break;
        }
        //jazda na dworzec pierwszy kurs krotszy
        int czas_dojazdu = (kursow == 0) ? losuj(1000, 2000) : losuj(8000, 15000);
        log_print(KOLOR_BUS, tag, "Wraca na dworzec (%dms). PID=%d", czas_dojazdu, getpid());
        
        //czekanie z mozliwoscia przerwania oparte na time()
        czas_start = time(NULL);
        czas_koniec = czas_start + (czas_dojazdu / 1000) + 1;
        while (time(NULL) < czas_koniec && bus_running) {
            if (czy_zakonczyc(shm)) break;
            //usleep(10000);//lub sched_yield();
        }
        if (!bus_running || czy_zakonczyc(shm)) {//sprawdzenie warunkow zakonczenia
            log_print(KOLOR_BUS, tag, "Warunki zakonczenia - koncze. PID=%d", getpid());
            break;
        }
        //proba zajecia przystanku (non-blocking)
        log_print(KOLOR_BUS, tag, "Czeka na wjazd na przystanek. PID=%d", getpid());
        struct sembuf zajmij_przystanek = {SEM_BUS_STOP, -1, SEM_UNDO};  //blokujacy!
        //Sprawdz przed zajmowaniem
        if (!shm->dworzec_otwarty || !bus_running) {
            log_print(KOLOR_BUS, tag, "Dworzec zamkniety - koncze. PID=%d", getpid());
            goto koniec;
        }
        //blokujace zajecie przystanku autobus przejdzie
        //Petla retry dla EINTR
        while (semop(sem_id, &zajmij_przystanek, 1) == -1) {
            if (errno == EINTR) {
                //Sprawdz czy to nie byl sygnal zakonczenia
                if (!bus_running || !shm->dworzec_otwarty) {
                    log_print(KOLOR_BUS, tag, "Dworzec zamkniety - koncze. PID=%d", getpid());
                    goto koniec;
                }//ponow probe
                continue;
            } else {
                perror("bus semop zajmij_peron");
                goto koniec;
            }
        }
        mam_przystanek = 1;//zajalem przystanek
        
        //sprawdz PO zajeciu czy dworzec nadal otwarty
        if (!shm->dworzec_otwarty || czy_zakonczyc(shm)) {
            log_print(KOLOR_BUS, tag, "Dworzec zamkniety po zajeciu przystanku koncze. PID=%d", getpid());
            semop(sem_id, &zwolnij_przystanek, 1);
            mam_przystanek = 0;
            goto koniec;
        }
        //autobus zajal przystanek - aktualizacja stanu
        kursow++;
        wymuszony_odjazd = 0;

        while (semop(sem_id, &shm_lock, 1) == -1) {
            if (errno == EINTR) continue;
            break;
        }
        shm->aktualny_bus_pid = getpid();
        shm->aktualny_bus_id = bus_id;
        shm->bus_na_przystanku = true;
        shm->miejsca_zajete = 0;
        shm->rowery_zajete = 0;
        while (semop(sem_id, &shm_unlock, 1) == -1 && errno == EINTR);

        log_print(KOLOR_BUS, tag, "[Kurs #%d] Na przystanku. PID=%d, Miejsca: 0/%d, Rowery: 0/%d",
                  kursow, getpid(), pojemnosc, rowery);

        //postój i przyjmowanie pasażerów
        int czas_na_przystanku = 0;//czas spedzony na przystanku
        int pasazerow_w_kursie = 0;

        if (!shm->dworzec_otwarty) {
            log_print(KOLOR_BUS, tag, "Dworzec zamkniety - nie przyjmuje pasazerow, koncze. PID=%d", getpid());
            //zwolnij przystanek i zakoncz nie jedz pustej trasy
            while (semop(sem_id, &shm_lock, 1) == -1) {
                if (errno == EINTR) continue;
                break;
            }
            shm->bus_na_przystanku = false;
            shm->aktualny_bus_pid = 0;
            shm->aktualny_bus_id = 0;
            while (semop(sem_id, &shm_unlock, 1) == -1 && errno == EINTR);
            semop(sem_id, &zwolnij_przystanek, 1);
            mam_przystanek = 0;
            break;
        } else {
            //petla przyjmowania pasazerow
            czas_start = time(NULL);  //start pomiaru czasu postoju
    
            while (czas_na_przystanku < czas_postoju && !wymuszony_odjazd && bus_running && shm->dworzec_otwarty) {
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
                        if (semop(sem_id, &shm_lock, 1) == 0) {
                            shm->odrzuconych_bez_biletu++;
                            semop(sem_id, &shm_unlock, 1);
                        }
                        //wyslij odmowe do pasazera
                        OdpowiedzMsg odp_bez = { .mtype = bilet.pid_pasazera, .przyjety = 0 };
                        msgsnd(msg_id, &odp_bez, sizeof(OdpowiedzMsg) - sizeof(long), 0);
                        continue;
                    }
                    log_print(KOLOR_BUS, tag, "Kierowca: PAS %d - bilet OK.", bilet.id_pasazera);
                    while (semop(sem_id, &shm_lock, 1) == -1) {
                        if (errno == EINTR) continue;
                        break;
                    }
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
                        shm->pasazerow_w_trasie += ile_osob;
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
                czas_na_przystanku = (int)((time(NULL) - czas_start) * 1000);
            }
            if (wymuszony_odjazd) {
                log_print(KOLOR_BUS, tag, ">>> WYMUSZONY ODJAZD (SIGUSR1)! PID=%d <<<", getpid());
            }//SIGUSR2 w trakcie zbierania jesli brak pasazerow zakoncz bez jazdy
            if (!shm->dworzec_otwarty && pasazerow_w_kursie == 0) {
                log_print(KOLOR_BUS, tag, "Dworzec zamkniety, brak pasazerow - koncze. PID=%d", getpid());
                while (semop(sem_id, &shm_lock, 1) == -1) {
                    if (errno == EINTR) continue;
                    break;
                }
                shm->bus_na_przystanku = false;
                shm->aktualny_bus_pid = 0;
                shm->aktualny_bus_id = 0;
                while (semop(sem_id, &shm_unlock, 1) == -1 && errno == EINTR);
                semop(sem_id, &zwolnij_przystanek, 1);
                mam_przystanek = 0;
                break;
            }
        }//sygnalizacja odjazdu pasazerowie w drzwiach zobacza ze bus odjechal i zwolnia semafor
        while (semop(sem_id, &shm_lock, 1) == -1) {
            if (errno == EINTR) continue;
            break;
        }
        shm->bus_na_przystanku = false;
        shm->aktualny_bus_pid = 0;
        shm->aktualny_bus_id = 0;
        while (semop(sem_id, &shm_unlock, 1) == -1 && errno == EINTR);
        
        //Odpowiedz odmownie wszystkim nieobsluzonym pasazerom (zeby nie czekali w nieskonczonosc)
        BiletMsg old;
        OdpowiedzMsg odmowa;
        odmowa.przyjety = 0;
        
        //VIP
        while (msgrcv(msg_id, &old, sizeof(BiletMsg) - sizeof(long), getpid(), IPC_NOWAIT) != -1) {
            odmowa.mtype = old.pid_pasazera;
            msgsnd(msg_id, &odmowa, sizeof(OdpowiedzMsg) - sizeof(long), IPC_NOWAIT);
        }//Zwykli
        while (msgrcv(msg_id, &old, sizeof(BiletMsg) - sizeof(long), getpid() + 1000000, IPC_NOWAIT) != -1) {
            odmowa.mtype = old.pid_pasazera;
            msgsnd(msg_id, &odmowa, sizeof(OdpowiedzMsg) - sizeof(long), IPC_NOWAIT);
        }
        //Czekaj az drzwi beda wolne (pasazer moze byc w trakcie wsiadania)
        log_print(KOLOR_BUS, tag, "Czeka az drzwi beda wolne. PID=%d", getpid());
        struct sembuf zablokuj_oba[2] = {
            {SEM_DOOR_NORMAL, -1, SEM_UNDO},
            {SEM_DOOR_ROWER, -1, SEM_UNDO}
        };
        while (semop(sem_id, zablokuj_oba, 2) == -1 && errno == EINTR);
        log_print(KOLOR_BUS, tag, "ZAMYKAM DRZWI PRZED ODJAZDEM. PID=%d", getpid());
        //aktualizacja stanu po odjedzie
        int w_trasie = 0;
        while (semop(sem_id, &shm_lock, 1) == -1) {
            if (errno == EINTR) continue;
            break;
        }
        przewiezionych += pasazerow_w_kursie;
        w_trasie = shm->pasazerow_w_trasie;
        while (semop(sem_id, &shm_unlock, 1) == -1 && errno == EINTR);

        log_print(KOLOR_BUS, tag, "ODJAZD! Zabral %d osob. W trasie: %d. PID=%d",
                  pasazerow_w_kursie, w_trasie, getpid());
        //zwolnienie peronu i otwarcie drzwi
        semop(sem_id, &odblokuj_drzwi_n, 1);
        semop(sem_id, &odblokuj_drzwi_r, 1);
        semop(sem_id, &zwolnij_przystanek, 1);
        mam_przystanek = 0;
        //jazda do miejsca docelowego
        log_print(KOLOR_BUS, tag, "Jedzie do miejsca docelowego (%dms). PID=%d", czas_trasy_Ti, getpid());
        
        //jazda do celu oparty na time()
        czas_start = time(NULL);
        czas_koniec = czas_start + (czas_trasy_Ti / 1000) + 1;
        while (time(NULL) < czas_koniec && bus_running) {
            if (!shm->symulacja_aktywna) break;
            //usleep(10000); //lub sched_yield();
        }
        //pasazerowie wysiadaja w miejscu docelowym KRYTYCZNE, blokujace
        {
            struct sembuf lock = {SEM_SHM, -1, SEM_UNDO};
            struct sembuf unlock = {SEM_SHM, 1, SEM_UNDO};
            while (semop(sem_id, &lock, 1) == -1) {
                if (errno == EINTR) continue;
                break;
            }
            shm->pasazerow_w_trasie -= pasazerow_w_kursie;
            shm->total_przewiezionych += pasazerow_w_kursie;
            while (semop(sem_id, &unlock, 1) == -1 && errno == EINTR);
        }
        log_print(KOLOR_BUS, tag, "Miejsce docelowe - pasazerowie wysiedli (%d). PID=%d", pasazerow_w_kursie, getpid());
        //koniec jesli dworzec zamkniety autobus konczy po rozwiezieniu pasazerow
        if (!shm->dworzec_otwarty) {
            log_print(KOLOR_BUS, tag, "Dworzec zamkniety - koncze prace po rozwiezieniu pasazerow. PID=%d", getpid());
            break;
        }
    }
koniec://koniec pracy autobusu
    //Zwolnij przystanek jesli autobus go trzyma
    if (mam_przystanek) {
        while (semop(sem_id, &shm_lock, 1) == -1) {
            if (errno == EINTR) continue;
            break;
        }
        shm->bus_na_przystanku = false;
        shm->aktualny_bus_pid = 0;
        shm->aktualny_bus_id = 0;
        while (semop(sem_id, &shm_unlock, 1) == -1 && errno == EINTR);
        semop(sem_id, &zwolnij_przystanek, 1);
        mam_przystanek = 0;
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
