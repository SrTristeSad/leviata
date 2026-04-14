/* ============================================================================
 * MOTOR LEVIATA - BIBLIOTECA PADRÃO (STD)
 * ============================================================================ */
#ifndef LEVIATA_STDLIB_H
#define LEVIATA_STDLIB_H

#include "leviata_vm.h"

/* Registra TODAS as funções da biblioteca padrão na VM */
void stdlib_register_all(LvVM *vm);

/* ---- Módulos Individuais ---- */
void stdlib_register_io(LvVM *vm);       /* entrada/saída, arquivos */
void stdlib_register_math(LvVM *vm);     /* matemática básica e avançada */
void stdlib_register_string(LvVM *vm);   /* manipulação de strings */
void stdlib_register_list(LvVM *vm);     /* operações em listas/arrays */
void stdlib_register_system(LvVM *vm);   /* tempo, sys, OS */
void stdlib_register_graphics(LvVM *vm); /* bindings de UI/Gráficos (FFI) */
void stdlib_register_game(LvVM *vm);     /* motor de jogos (raylib/SDL stub) */

#endif /* LEVIATA_STDLIB_H */
