// utils.c - Funkcje pomocnicze, Projekt SO 2025/2026 - Temat 12
#include "common.h"

// Zmienne globalne IPC
int sem_id = -1;
int shm_id = -1;
int msg_id = -1;

// get_timestamp - Timestamp w formacie HH:MM:SS
void get_timestamp(char* buf, size_t size) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buf, size, "%H:%M:%S", t);
}

// log_print - Logowanie na ekran (kolory) i do pliku, WYMAGANE: open(), write(), close(), semop()
void log_print(const char* kolor, const char* tag, const char* fmt, ...) {
    struct sembuf lock = {SEM_LOG, -1, 0};
    struct sembuf unlock = {SEM_LOG, 1, 0};

    // Sekcja krytyczna 
    if (sem_id != -1) {
        semop(sem_id, &lock, 1);
    }

    // Timestamp 
    char time_buf[16];
    get_timestamp(time_buf, sizeof(time_buf));

    // Formatowanie wiadomości 
    char msg[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf(msg, sizeof(msg), fmt, args);
    va_end(args);

    //EKRAN (z kolorami)
    char screen[700];
    int slen = snprintf(screen, sizeof(screen),
        "%s[%s]%s [%-6s] %s%s\n",
        KOLOR_MAIN, time_buf, kolor, tag, msg, KOLOR_RESET);
    write(STDOUT_FILENO, screen, slen);

    //PLIK (bez kolorów) raport.txt
    char file_line[600];
    int flen = snprintf(file_line, sizeof(file_line),
                        "[%s] [%-6s] %s\n",
                        time_buf, tag, msg);

    // WYMAGANIE 5.2.a: open(), write(), close() 
    int fd = open("raport.txt", O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd != -1) {
        write(fd, file_line, flen);
        close(fd);
    }

    // Zwolnienie 
    if (sem_id != -1) {
        semop(sem_id, &unlock, 1);
    }
}

// losuj - Losowa liczba z zakresu [min, max]
int losuj(int min, int max) {
    if (max <= min) return min;
    return min + rand() % (max - min + 1);
}

// msleep - Uspienie na ms milisekund
void msleep(int ms) {
    usleep(ms * 1000);
}
