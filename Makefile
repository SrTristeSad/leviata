# ============================================================================
# LEVIATA 3 — Makefile
# ============================================================================

CC      ?= gcc
CFLAGS  ?= -std=c99 -Wall -Wextra -Wpedantic -Iinclude
LDFLAGS ?= -lm
TARGET  ?= leviata

SRC = src/main.c \
      src/leviata_lexer.c \
      src/leviata_ast.c \
      src/leviata_parser_v39.c \
      src/leviata_vm.c \
      src/leviata_compiler_v35.c \
      src/leviata_compiler_v38.c \
      src/leviata_stdlib.c \
      src/leviata_ffi.c \
      src/leviata_2d.c \
      src/leviata_world.c \
      src/leviata_canon.c \
      src/leviata_resources.c \
      src/leviata_classes.c \
      src/leviata_super.c \
      src/leviata_maps_v39.c \
      src/leviata_runtime_v39.c \
      src/leviata_canon_finalize.c

RAYLIB_CFLAGS := $(shell pkg-config --cflags raylib 2>/dev/null)
RAYLIB_LIBS   := $(shell pkg-config --libs raylib 2>/dev/null)
ifeq ($(strip $(RAYLIB_LIBS)),)
RAYLIB_LIBS = -lraylib
endif

all:
	$(CC) $(CFLAGS) -O2 $(SRC) -o $(TARGET) $(LDFLAGS)
	@echo ""
	@echo "✓ Leviata compilado: ./$(TARGET)"
	@echo "  Para jogos com janela real: make raylib"

debug:
	$(CC) $(CFLAGS) -g -O0 -fsanitize=address,undefined $(SRC) -o $(TARGET)_debug $(LDFLAGS)
	@echo "✓ Build de debug: ./$(TARGET)_debug"

raylib:
	$(CC) $(CFLAGS) $(RAYLIB_CFLAGS) -O2 -DLVA_USE_RAYLIB $(SRC) -o $(TARGET) $(LDFLAGS) $(RAYLIB_LIBS)
	@echo ""
	@echo "✓ Leviata + Raylib compilado: ./$(TARGET)"
	@echo "  Teste base: ./$(TARGET) exemplos/jogo.pt"
	@echo "  Motor 2D:  ./$(TARGET) exemplos/motor2d.pt"
	@echo "  World:     ./$(TARGET) exemplos/mundo.pt"

raylib-demo: raylib
	./$(TARGET) exemplos/mundo.pt

sdl2:
	$(CC) $(CFLAGS) -O2 -DLVA_USE_SDL2 $(SRC) -o $(TARGET) $(LDFLAGS) -lSDL2
	@echo "✓ Leviata + SDL2 compilado: ./$(TARGET)"

demo: all
	./$(TARGET) --demo

clean:
	rm -f $(TARGET) $(TARGET)_debug

.PHONY: all debug raylib raylib-demo sdl2 demo clean
