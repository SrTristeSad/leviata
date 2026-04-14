# ============================================================================
# MOTOR LEVIATA v2.0 — Makefile
# ============================================================================

CC      = gcc
CFLAGS  = -std=c99 -Wall -Wextra -Wpedantic -Iinclude
LDFLAGS = -lm
TARGET  = leviata

SRC = src/main.c \
      src/leviata_lexer.c \
      src/leviata_ast.c \
      src/leviata_parser.c \
      src/leviata_vm.c \
      src/leviata_compiler.c \
      src/leviata_stdlib.c \
      src/leviata_ffi.c

# ---- Build padrão (sem backend gráfico) ----
all:
	$(CC) $(CFLAGS) -O2 $(SRC) -o $(TARGET) $(LDFLAGS)
	@echo ""
	@echo "✓ Motor Leviata compilado com sucesso: ./$(TARGET)"
	@echo ""
	@echo "  Uso:"
	@echo "    ./$(TARGET) --demo            Demonstração completa"
	@echo "    ./$(TARGET) --ast arquivo.pt  Mostra a AST"
	@echo "    ./$(TARGET) arquivo.pt        Executa programa PT"
	@echo "    ./$(TARGET)                   REPL interativo"

# ---- Build com debug e sanitizers ----
debug:
	$(CC) $(CFLAGS) -g -fsanitize=address,undefined $(SRC) -o $(TARGET)_debug $(LDFLAGS)
	@echo "✓ Build de debug: ./$(TARGET)_debug"

# ---- Build com Raylib ----
raylib:
	$(CC) $(CFLAGS) -O2 -DLVA_USE_RAYLIB $(SRC) -o $(TARGET) $(LDFLAGS) -lraylib
	@echo "✓ Motor Leviata + Raylib compilado: ./$(TARGET)"

# ---- Build com SDL2 ----
sdl2:
	$(CC) $(CFLAGS) -O2 -DLVA_USE_SDL2 $(SRC) -o $(TARGET) $(LDFLAGS) -lSDL2
	@echo "✓ Motor Leviata + SDL2 compilado: ./$(TARGET)"

# ---- Executar demo automaticamente ----
demo: all
	./$(TARGET) --demo

# ---- Limpar ----
clean:
	rm -f $(TARGET) $(TARGET)_debug

.PHONY: all debug raylib sdl2 demo clean
