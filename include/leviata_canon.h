/* ============================================================================
 * LEVIATA 3 - SUPERFÍCIE CANÔNICA DA BIBLIOTECA
 * ============================================================================ */
#ifndef LEVIATA_CANON_H
#define LEVIATA_CANON_H

#include <stdlib.h>
#include "leviata_vm.h"

/* Registra aliases e funções públicas canônicas. */
void leviata_canon_register(LvVM *vm);

/*
 * Reconstrói a tabela global depois da migração, removendo nomes legados de
 * forma segura mesmo com colisões de hash do open addressing.
 */
void leviata_canon_finalize(LvVM *vm);

#endif /* LEVIATA_CANON_H */
