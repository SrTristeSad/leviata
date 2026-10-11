/* ============================================================================
 * LEVIATA 3 - SUPERFÍCIE CANÔNICA DA BIBLIOTECA
 * ============================================================================ */
#ifndef LEVIATA_CANON_H
#define LEVIATA_CANON_H

#include "leviata_vm.h"

/*
 * Registra a API pública do Leviata 3 depois da stdlib, FFI, 2D e World.
 * Implementações internas podem manter nomes históricos em C; o código .pt
 * enxerga somente a superfície canônica registrada aqui.
 */
void leviata_canon_register(LvVM *vm);

#endif /* LEVIATA_CANON_H */
