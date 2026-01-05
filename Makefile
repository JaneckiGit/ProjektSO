# # Makefile - Projekt SO 2025/2026 - Autobus Podmiejski
# # 
# # Uzycie:
# #   make        - kompilacja wszystkiego
# #   make run    - uruchomienie (domyslne parametry)
# #   make clean  - czyszczenie
# #   make clean-ipc - czyszczenie zasobow IPC

CC = gcc
CFLAGS = -Wall -Wextra -pthread -I./include
LDFLAGS = -pthread

SRC_DIR = src
BIN_DIR = bin

# Programy
TARGET = $(BIN_DIR)/autobus_main
TARGET_BUS = $(BIN_DIR)/autobus
TARGET_KASA = $(BIN_DIR)/kasa
TARGET_PAS = $(BIN_DIR)/pasazer

# Regula domyslna
all: directories $(TARGET) $(TARGET_BUS) $(TARGET_KASA) $(TARGET_PAS)
	@echo "Kompilacja zakonczona"
	@echo "Uruchom: ./$(TARGET) [N] [P] [R] [T] [K]"

directories:
	@mkdir -p $(BIN_DIR)

# Glowny program (dyspozytor)
$(TARGET): $(SRC_DIR)/main.c $(SRC_DIR)/dyspozytor.c $(SRC_DIR)/utils.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

# Autobus (osobny program dla exec)
$(TARGET_BUS): $(SRC_DIR)/bus.c $(SRC_DIR)/utils.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

# Kasa (osobny program dla exec)
$(TARGET_KASA): $(SRC_DIR)/kasa.c $(SRC_DIR)/utils.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

# Pasazer (osobny program dla exec)
$(TARGET_PAS): $(SRC_DIR)/pasazer.c $(SRC_DIR)/utils.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

# Uruchomienie (czyści IPC przed startem)
run: all
	@ipcrm -a 2>/dev/null || true
	./$(TARGET)

# Czyszczenie plików
clean:
	rm -rf $(BIN_DIR) raport.txt

# Czyszczenie zasobów IPC
clean-ipc:
	@ipcs -s | grep `whoami` | awk '{print $$2}' | xargs -I {} ipcrm -s {} 2>/dev/null || true
	@ipcs -m | grep `whoami` | awk '{print $$2}' | xargs -I {} ipcrm -m {} 2>/dev/null || true
	@ipcs -q | grep `whoami` | awk '{print $$2}' | xargs -I {} ipcrm -q {} 2>/dev/null || true
	@echo "Zasoby IPC wyczyszczone"


# Testy
test1: all
	@./tests/test.sh 1

test2: all
	@./tests/test.sh 2

test3: all
	@./tests/test.sh 3

test4: all
	@./tests/test.sh 4

test-all: all
	@./tests/test.sh all

.PHONY: all directories run clean clean-ipc
.PHONY: all directories run clean clean-ipc test1 test2 test3 test4 test-all