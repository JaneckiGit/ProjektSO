# Makefile - Projekt SO 2025/2026 - Autobus Podmiejski
# 
# Użycie:
#   make        - kompilacja
#   make run    - uruchomienie
#   make clean  - czyszczenie
#   make test   - uruchomienie testów

CC = gcc
CFLAGS = -Wall -Wextra -pthread -I./include
LDFLAGS = -pthread

SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

TARGET = $(BIN_DIR)/autobus
TARGET_KASA = $(BIN_DIR)/kasa

# Pliki źródłowe
SRCS = $(SRC_DIR)/main.c \
       $(SRC_DIR)/dyspozytor.c \
       $(SRC_DIR)/bus.c \
       $(SRC_DIR)/kasa.c \
       $(SRC_DIR)/pasazer.c \
       $(SRC_DIR)/utils.c

# Pliki obiektowe
OBJS = $(OBJ_DIR)/main.o \
       $(OBJ_DIR)/dyspozytor.o \
       $(OBJ_DIR)/bus.o \
       $(OBJ_DIR)/kasa.o \
       $(OBJ_DIR)/pasazer.o \
       $(OBJ_DIR)/utils.o

# Reguła domyślna
all: directories $(TARGET) $(TARGET_KASA)
	@echo "Kompilacja zakończona: $(TARGET)"
	@echo "Uruchom: ./$(TARGET) [N] [P] [R] [T]"

# Tworzenie katalogów
directories:
	@mkdir -p $(OBJ_DIR) $(BIN_DIR)

# Linkowanie
$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

# Kompilacja plików .c do .o
$(TARGET_KASA): $(SRC_DIR)/kasa.c $(SRC_DIR)/utils.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

# Zależności od nagłówków
$(OBJ_DIR)/main.o: $(SRC_DIR)/main.c include/common.h include/dyspozytor.h
$(OBJ_DIR)/dyspozytor.o: $(SRC_DIR)/dyspozytor.c include/common.h include/dyspozytor.h include/bus.h include/kasa.h include/pasazer.h
$(OBJ_DIR)/bus.o: $(SRC_DIR)/bus.c include/common.h include/bus.h
$(OBJ_DIR)/kasa.o: $(SRC_DIR)/kasa.c include/common.h include/kasa.h
$(OBJ_DIR)/pasazer.o: $(SRC_DIR)/pasazer.c include/common.h include/pasazer.h
$(OBJ_DIR)/utils.o: $(SRC_DIR)/utils.c include/common.h

# Uruchomienie
run: all
	./$(TARGET)

# Uruchomienie z parametrami
run-custom: all
	./$(TARGET) 3 10 3 5000

# Testy
test: all
	@chmod +x tests/test.sh
	@./tests/test.sh

# Czyszczenie
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR) raport.txt

# Czyszczenie zasobów IPC (jeśli zostały)
clean-ipc:
	@echo "Czyszczenie zasobów IPC..."
	@ipcs -s | grep `whoami` | awk '{print $$2}' | xargs -I {} ipcrm -s {} 2>/dev/null || true
	@ipcs -m | grep `whoami` | awk '{print $$2}' | xargs -I {} ipcrm -m {} 2>/dev/null || true
	@ipcs -q | grep `whoami` | awk '{print $$2}' | xargs -I {} ipcrm -q {} 2>/dev/null || true
	@echo "Gotowe."

.PHONY: all directories run run-custom test clean clean-ipc
