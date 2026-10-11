/* ============================================================================
 * LEVIATA 3.6 - EXECUÇÃO DE ARQUIVOS E MÓDULOS
 * ============================================================================ */
#ifndef LEVIATA_RUNTIME_H
#define LEVIATA_RUNTIME_H

#include "leviata_vm.h"

#define LV_MODULE_MAX 256
#define LV_MODULE_PATH_MAX 1024

typedef struct LvModuleExports {
    char path[LV_MODULE_PATH_MAX];
    char (*names)[LV_MAX_IDENTIFIER];
    int count;
    int capacity;
    struct LvModuleExports *next;
} LvModuleExports;

typedef struct {
    char loaded[LV_MODULE_MAX][LV_MODULE_PATH_MAX];
    int loaded_count;
    char loading[LV_MODULE_MAX][LV_MODULE_PATH_MAX];
    int loading_count;
    LvModuleExports *exports;
} LvRuntime;

void leviata_runtime_init(LvRuntime *rt);
void leviata_runtime_free(LvRuntime *rt);

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
