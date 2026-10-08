# ============================================================================
# Makefile — Build helper for Connect Four (Puissance 4)
# Cross-platform support for Windows (MinGW) and Linux / macOS
# ============================================================================

CC       = gcc
CFLAGS   = -std=c11 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -O2
LDFLAGS  =
SOURCES  = main.c project.c
OBJECTS  = $(SOURCES:.c=.o)
HEADERS  = puissance4.h

ifeq ($(OS),Windows_NT)
    TARGET = puissance4.exe
    RM     = del /F /Q 2>NUL || rem
else
    TARGET = puissance4
    RM     = rm -f
endif

# Default target
all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Every object depends on the header: editing it rebuilds everything
$(OBJECTS): $(HEADERS)

# Run the program
run: $(TARGET)
	./$(TARGET)

# Debug build with sanitizers
debug: CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -g -O0 -fsanitize=address,undefined
debug: LDFLAGS = -fsanitize=address,undefined
debug: clean $(TARGET)

# ASCII-only build (no Unicode borders)
ascii: CFLAGS += -DASCII_ONLY
ascii: clean $(TARGET)

# Clean build artifacts
clean:
	-$(RM) $(TARGET) $(OBJECTS)

.PHONY: all run debug ascii clean
