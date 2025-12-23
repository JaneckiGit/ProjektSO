//kasa.c - Symulacja kasy biletowej

#include "common.h"
#include "kasa.h"

void proces_kasa(int semid, int shmid, int msgid) {
    SharedState *state = (SharedState *)shmat(shmid, NULL, 0);
    if (state == (void *)-1) {
        perror("Kasa: shmat error");
        exit(1);
    }
