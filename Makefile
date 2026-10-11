# ============================================================================
# MOTOR LEVIATA v2.1 — Makefile
# ============================================================================

CC      ?= gcc
CFLAGS  ?= -std=c99 -Wall -Wextra -Wpedantic -Iinclude
LDFLAGS ?= -lm
TARGET  ?= leviata

SRC = src/main.c \
      src/leviata_lexer.c \
      src/leviata_ast.c \
      src/leviata_parser.c \
      src/leviata_vm.c \
      src/leviata_compiler.c \
      src/leviata_stdlib.c \
      src/leviata_ffi.c

# Tenta usar pkg-config para encontrar Raylib. Se nao existir, cai em -lraylib.
RAYLIB_CFLAGS := $(shell pkg-config --cflags raylib 2>/dev/null)
RAYLIB_LIBS   := $(shell pkg-config --libs raylib 2>/dev/null)
ifeq ($(strip $(RAYLIB_LIBS)),)
RAYLIB_LIBS = -lraylib
endif

# ---- Build padrao (VM + stdlib, sem dependencia grafica) ----
all:
	$(CC) $(CFLAGS) -O2 $(SRC) -o $(TARGET) $(LDFLAGS)
	@echo ""
	@echo "✓ Motor Leviata compilado: ./$(TARGET)"
	@echo "  Para jogos com janela real: make raylib"

# ---- Build com debug e sanitizers ----
debug:
	$(CC) $(CFLAGS) -g -O0 -fsanitize=address,undefined $(SRC) -o $(TARGET)_debug $(LDFLAGS)
	@echo "✓ Build de debug: ./$(TARGET)_debug"

# ---- Build com Raylib real ----
raylib:
	$(CC) $(CFLAGS) $(RAYLIB_CFLAGS) -O2 -DLVA_USE_RAYLIB $(SRC) -o $(TARGET) $(LDFLAGS) $(RAYLIB_LIBS)
	@echo ""
	@echo "✓ Leviata + Raylib compilado: ./$(TARGET)"
	@echo "  Teste: ./$(TARGET) exemplos/jogo.pt"

# ---- Compila e executa o exemplo grafico ----
raylib-demo: raylib
	./$(TARGET) exemplos/jogo.pt

# ---- SDL2 permanece como backend futuro/experimental ----
sdl2:
	$(CC) $(CFLAGS) -O2 -DLVA_USE_SDL2 $(SRC) -o $(TARGET) $(LDFLAGS) -lSDL2
	@echo "✓ Leviata + SDL2 compilado: ./$(TARGET)"

# ---- Demonstracao da linguagem ----
demo: all
	./$(TARGET) --demo

# ---- Limpar ----
clean:
	rm -f $(TARGET) $(TARGET)_debug

.PHONY: all debug raylib raylib-demo sdl2 demo clean
