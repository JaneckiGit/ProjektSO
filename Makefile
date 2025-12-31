# # Makefile - Projekt SO 2025/2026 - Autobus Podmiejski
# # 
# # Uzycie:
# #   make        - kompilacja wszystkiego
# #   make run    - uruchomienie (domyslne parametry)
# #   make clean  - czyszczenie

# CC = gcc
# CFLAGS = -Wall -Wextra -pthread -I./include
# LDFLAGS = -pthread

# SRC_DIR = src
# OBJ_DIR = obj
# BIN_DIR = bin

# # Glowny program
# TARGET = $(BIN_DIR)/autobus_main

# # Osobne programy dla exec()
# TARGET_BUS = $(BIN_DIR)/autobus
# TARGET_KASA = $(BIN_DIR)/kasa
# TARGET_PAS = $(BIN_DIR)/pasazer

# # Regula domyslna - kompiluj wszystko
# all: directories $(TARGET) $(TARGET_BUS) $(TARGET_KASA) $(TARGET_PAS)
# 	@echo ""
# 	@echo "=== Kompilacja zakonczona ==="
# 	@echo "Glowny program: $(TARGET)"
# 	@echo "Programy exec:  $(TARGET_BUS), $(TARGET_KASA), $(TARGET_PAS)"
# 	@echo ""
# 	@echo "Uruchom: ./$(TARGET) [N] [P] [R] [T] [K]"
# 	@echo "  N - liczba autobusow (domyslnie 3)"
# 	@echo "  P - pojemnosc autobusu (domyslnie 10)"
# 	@echo "  R - miejsca na rowery (domyslnie 3)"
# 	@echo "  T - czas postoju w ms (domyslnie 5000)"
# 	@echo "  K - liczba kas (domyslnie 2)"

# # Tworzenie katalogow
# directories:
# 	@mkdir -p $(OBJ_DIR) $(BIN_DIR)

# # Glowny program
# $(TARGET): $(SRC_DIR)/main.c $(SRC_DIR)/dyspozytor.c $(SRC_DIR)/utils.c
# 	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

# # Program autobusu (dla exec)
# $(TARGET_BUS): $(SRC_DIR)/bus.c $(SRC_DIR)/utils.c
# 	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

# # Program kasy (dla exec) - kazda kasa to osobny proces
# $(TARGET_KASA): $(SRC_DIR)/kasa.c $(SRC_DIR)/utils.c
# 	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

# # Program pasazera (dla exec)
# $(TARGET_PAS): $(SRC_DIR)/pasazer.c $(SRC_DIR)/utils.c
# 	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

# # Uruchomienie z domyslnymi parametrami
# run: all
# 	./$(TARGET)

# # Uruchomienie z parametrami
# run-custom: all
# 	./$(TARGET) 3 10 3 5000 2

# # Testy
# test: all
# 	@chmod +x tests/test.sh
# 	@./tests/test.sh

# # Czyszczenie
# clean:
# 	rm -rf $(OBJ_DIR) $(BIN_DIR) raport.txt

# # Czyszczenie zasobow IPC
# clean-ipc:
# 	@echo "Czyszczenie zasobow IPC..."
# 	@ipcs -s | grep `whoami` | awk '{print $$2}' | xargs -I {} ipcrm -s {} 2>/dev/null || true
# 	@ipcs -m | grep `whoami` | awk '{print $$2}' | xargs -I {} ipcrm -m {} 2>/dev/null || true
# 	@ipcs -q | grep `whoami` | awk '{print $$2}' | xargs -I {} ipcrm -q {} 2>/dev/null || true
# 	@echo "Gotowe."

# .PHONY: all directories run run-custom test clean clean-ipc

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

run: all
	./$(TARGET)

clean:
	rm -rf $(BIN_DIR) raport.txt

clean-ipc:
	@ipcs -s | grep `whoami` | awk '{print $$2}' | xargs -I {} ipcrm -s {} 2>/dev/null || true
	@ipcs -m | grep `whoami` | awk '{print $$2}' | xargs -I {} ipcrm -m {} 2>/dev/null || true
	@ipcs -q | grep `whoami` | awk '{print $$2}' | xargs -I {} ipcrm -q {} 2>/dev/null || true

.PHONY: all directories run clean clean-ipc