/* ============================================================================
 * LEVIATA 3.3 - EXECUÇÃO DE ARQUIVOS E MÓDULOS
 * ============================================================================ */
#ifndef LEVIATA_RUNTIME_H
#define LEVIATA_RUNTIME_H

#include "leviata_vm.h"

#define LV_MODULE_MAX 256
#define LV_MODULE_PATH_MAX 1024

typedef struct {
    char loaded[LV_MODULE_MAX][LV_MODULE_PATH_MAX];
    int loaded_count;
    char loading[LV_MODULE_MAX][LV_MODULE_PATH_MAX];
    int loading_count;
} LvRuntime;

void leviata_runtime_init(LvRuntime *rt);

int leviata_execute_source(LvRuntime *rt,
                           LvVM *vm,
                           const char *source,
                           const char *origin,
                           int show_ast);

int leviata_execute_file(LvRuntime *rt,
                         LvVM *vm,
                         const char *path,
                         int show_ast);

#endif /* LEVIATA_RUNTIME_H */