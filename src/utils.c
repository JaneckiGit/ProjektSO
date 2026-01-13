//Funkcje pomocnicze
//zmienne globalne IPC
//funkcje Logow 
//funkcje pomocnicze (losuj, msleep)
//Inicjalizacja IPC dla procesow potomnych
#include "common.h"

// Zmienne globalne IPC
int sem_id = -1;
int shm_id = -1;
int msg_id = -1;
int msg_kasa_id = -1;
int msg_odp_id = -1;  //Kolejka odpowiedzi autobus -> pasazer

//get_timestamp - Timestamp w formacie HH:MM:SS
void get_timestamp(char* buf, size_t size) {
    setenv("TZ", "Europe/Warsaw", 1); 
    tzset();                            
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buf, size, "%H:%M:%S", t);
}
//log_print - Logowanie na ekran kolory i do pliku, WYMAGANIA: open(), write(), close(), semop()
void log_print(const char* kolor, const char* tag, const char* fmt, ...) {
    struct sembuf lock = {SEM_LOG, -1, SEM_UNDO};
    struct sembuf unlock = {SEM_LOG, 1, SEM_UNDO};  
    //sekcja krytyczna 
    if (sem_id != -1) {
        semop(sem_id, &lock, 1);
    }
    //Timestamp 
    char time_buf[16];
    get_timestamp(time_buf, sizeof(time_buf));

    //Formatowanie wiadomosci
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

    //PLIK raport.txt
    char file_line[600];
    int flen = snprintf(file_line, sizeof(file_line),
                        "[%s] [%-6s] %s\n",
                        time_buf, tag, msg);

    //raport.txt
    int fd = open("raport.txt", O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd != -1) {
        write(fd, file_line, flen);
        close(fd);
    }
    //zwolnienie 
    if (sem_id != -1) {
        semop(sem_id, &unlock, 1);
    }
}
//losuj losowa liczba z zakresu [min, max]
int losuj(int min, int max) {
    if (max <= min) return min;
    return min + rand() % (max - min + 1);
}
//msleep uspienie na ms milisekund
void msleep(int ms) {
    usleep(ms * 1000);
}
//Inicjalizacja IPC dla procesow potomnych (bus, kasa, pasazer)
//laczy się do ISTNIEJĄCYCH zasobów (bez IPC_CREAT
//Wywolywane przez: bus, kasa, pasazer
int init_ipc_client(void) {
    // Generowanie kluczy (identyczne jak w dyspozytorze)
    key_t key_sem = ftok(".", 'S');
    key_t key_shm = ftok(".", 'M');
    key_t key_msg = ftok(".", 'Q');
    key_t key_msg_kasa = ftok(".", 'K');
    key_t key_msg_odp = ftok(".", 'O');  //kolejka odpowiedzi
    
    if (key_sem == -1 || key_shm == -1 || key_msg == -1 || key_msg_kasa == -1 || key_msg_odp == -1) { 
        perror("ftok"); 
        return -1; 
    }
    //laczenie do istniejacych zasobow
    sem_id = semget(key_sem, 0, 0600);
    shm_id = shmget(key_shm, 0, 0600);
    msg_id = msgget(key_msg, 0600);
    msg_kasa_id = msgget(key_msg_kasa, 0600);
    msg_odp_id = msgget(key_msg_odp, 0600);
    
    if (sem_id == -1 || shm_id == -1 || msg_id == -1 || msg_kasa_id == -1 || msg_odp_id == -1) { 
        perror("IPC get"); 
        return -1; 
    }
    return 0;
}
