// pasazer.c - Modul pasazera, Projekt SO 2025/2026 - Temat 12
#include "common.h"
#include "pasazer.h"

void proces_pasazer(int id_pas) {
    srand(time(NULL) ^ getpid());

    char tag[16];
    snprintf(tag, sizeof(tag), "PAS %d", id_pas);

        // Podlaczenie do pamieci
    SharedData *shm = (SharedData *)shmat(shm_id, NULL, 0);
    if (shm == (void *)-1) {
        perror("pasazer shmat");
        exit(1);
    }

        // Sprawdz czy stacja otwarta
    if (!shm->stacja_otwarta) {
        log_print(KOLOR_PAS, tag, "Dworzec zamknięty - nie wchodzę. PID=%d", getpid());
        shmdt(shm);
        exit(0);
    }

        // GENEROWANIE ATRUBYTOW
    int wiek = losuj(1, 80);
    int czy_vip = (losuj(1, 100) == 1);     /* 1% VIP */
    int czy_rower = (losuj(1, 100) <= 15);  /* 15% z rowerem */
    int wiek_dziecka = 0;
    int ile_miejsc = 1;

        // Dziecko <8 lat musi miec opiekuna
    if (wiek < 8) {
            // To jest dziecko - musi byc z rodzicem
        wiek_dziecka = wiek;
        wiek = losuj(25, 50);  /* Rodzic */
        ile_miejsc = 2;
        czy_rower = 0;  /* Z dzieckiem bez roweru */
    } else if (wiek >= 25 && wiek <= 50 && losuj(1, 100) <= 20) {
            // Dorosly moze miec dziecko (20% szans)
        wiek_dziecka = losuj(1, 7);
        ile_miejsc = 2;
        czy_rower = 0;
    }

        // Aktualizacja statystyk
    struct sembuf shm_lock = {SEM_SHM, -1, 0};
    struct sembuf shm_unlock = {SEM_SHM, 1, 0};

    semop(sem_id, &shm_lock, 1);
    shm->total_pasazerow++;
    shm->pasazerow_czeka++;
    if (czy_vip) shm->vip_count++;
    semop(sem_id, &shm_unlock, 1);

        // Logowanie pojawienia sie
    if (wiek_dziecka > 0) {
        log_print(KOLOR_PAS, tag, 
                  "Przybył. PID=%d, Wiek=%d, Dziecko=%d lat, VIP=%d, Rower=%d",
                  getpid(), wiek, wiek_dziecka, czy_vip, czy_rower);
    } else {
        log_print(KOLOR_PAS, tag, 
                  "Przybył. PID=%d, Wiek=%d, VIP=%d, Rower=%d",
                  getpid(), wiek, czy_vip, czy_rower);
    }

        // KUPNO BILETU W KASIE
    if (!czy_vip) {
        /* Wybór okienka (losowo 1 lub 2) */
        int sem_kasa = (losuj(1, 2) == 1) ? SEM_KASA_1 : SEM_KASA_2;
        struct sembuf zajmij_kase = {sem_kasa, -1, 0};
        struct sembuf zwolnij_kase = {sem_kasa, 1, 0};

        log_print(KOLOR_PAS, tag, "Czeka w kolejce do kasy. PID=%d", getpid());

        if (semop(sem_id, &zajmij_kase, 1) == -1) {
            shmdt(shm);
            exit(1);
        }

        /* Rejestracja w kasie */
        semop(sem_id, &shm_lock, 1);
        if (shm->registered_count < MAX_REGISTERED) {
            shm->registered_pids[shm->registered_count] = getpid();
            shm->registered_wiek[shm->registered_count] = wiek;
            shm->registered_count++;
        }
        shm->sprzedanych_biletow += ile_miejsc;
        semop(sem_id, &shm_unlock, 1);

        /* Symulacja czasu zakupu */
        msleep(losuj(200, 500));

        semop(sem_id, &zwolnij_kase, 1);

        log_print(KOLOR_KASA, "KASA", 
                  "Zarejestrowano PAS %d (PID=%d, wiek=%d)", id_pas, getpid(), wiek);
        log_print(KOLOR_PAS, tag, "Ma bilet. Czeka na autobus. PID=%d", getpid());
    } else {
        log_print(KOLOR_PAS, tag, "(VIP) Omija kolejkę! PID=%d", getpid());
        
        /* VIP też rejestrowany */
        semop(sem_id, &shm_lock, 1);
        if (shm->registered_count < MAX_REGISTERED) {
            shm->registered_pids[shm->registered_count] = getpid();
            shm->registered_wiek[shm->registered_count] = wiek;
            shm->registered_count++;
        }
        semop(sem_id, &shm_unlock, 1);
    }

        // OCZEKIWANIE NA AUTOBUS
    int sem_drzwi = czy_rower ? SEM_DOOR_ROWER : SEM_DOOR_NORMAL;
    int proby = 0;
    const int MAX_PROB = 200;  /* ~20 sekund max */

    while (shm->stacja_otwarta && shm->symulacja_aktywna && proby < MAX_PROB) {
        proby++;

        /* Sprawdź czy jest autobus */
        if (shm->bus_na_peronie && shm->aktualny_bus_pid > 0) {
            struct sembuf wejdz = {sem_drzwi, -1, IPC_NOWAIT};
            struct sembuf wyjdz = {sem_drzwi, 1, 0};

            if (semop(sem_id, &wejdz, 1) == 0) {
                /* W drzwiach - sprawdź czy autobus nadal jest */
                if (shm->aktualny_bus_pid > 0) {
                    /* Przygotuj bilet */
                    BiletMsg bilet;
                    bilet.mtype = shm->aktualny_bus_pid;
                    bilet.pid_pasazera = getpid();
                    bilet.id_pasazera = id_pas;
                    bilet.wiek = wiek;
                    bilet.wiek_dziecka = wiek_dziecka;
                    bilet.czy_rower = czy_rower;
                    bilet.czy_vip = czy_vip;
                    bilet.ile_miejsc = ile_miejsc;

                    /* Wyślij bilet */
                    if (msgsnd(msg_id, &bilet, sizeof(BiletMsg) - sizeof(long), 0) == 0) {
                        /* Aktualizacja */
                        semop(sem_id, &shm_lock, 1);
                        shm->pasazerow_czeka--;
                        semop(sem_id, &shm_unlock, 1);

                        /* Zwolnij drzwi */
                        semop(sem_id, &wyjdz, 1);

                        shmdt(shm);
                        exit(0);  /* Sukces! */
                    }
                }
                semop(sem_id, &wyjdz, 1);
            }

            /* VIP czeka krócej */
            usleep(czy_vip ? 50000 : 100000);
        } else {
            usleep(200000);
        }
    }

        // REZYGNACJA
    const char* powod = !shm->stacja_otwarta ? "dworzec zamknięty" : 
                        !shm->symulacja_aktywna ? "koniec symulacji" : "timeout";
    
    log_print(KOLOR_PAS, tag, "Rezygnuje (%s). PID=%d", powod, getpid());

    semop(sem_id, &shm_lock, 1);
    shm->pasazerow_czeka--;
    semop(sem_id, &shm_unlock, 1);

    shmdt(shm);
    exit(0);
}
