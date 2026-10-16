CC = gcc
CFLAGS = -O2 -Wall

BUILD_DIR = build
TARGET = $(BUILD_DIR)/emu

SRCS = src/emulator.c

all: $(TARGET)

$(TARGET): $(BUILD_DIR)/emulator.o
	$(CC) $(CFLAGS) -o $@ $^ -lSDL2

$(BUILD_DIR)/emulator.o: src/emulator.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
