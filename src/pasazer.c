//pasazer.c - Symulacja pasażera

#include "common.h"
#include "pasazer.h"

void proces_pasazer(int id, int semid, int shmid, int msgid) {
    SharedState *state = (SharedState *)shmat(shmid, NULL, 0);
    if (state == (void *)-1) {
        perror("Pasazer: shmat error");
        exit(1);
    }