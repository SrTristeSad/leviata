/* ============================================================================
 * LEVIATA 3.7 - EXECUÇÃO, LINKEDIÇÃO E MÓDULOS ISOLADOS
 * ============================================================================ */
#ifndef LEVIATA_RUNTIME_H
#define LEVIATA_RUNTIME_H

#include "leviata_vm.h"

#define LV_MODULE_MAX 256
#define LV_MODULE_PATH_MAX 1024
#define LV_MODULE_SYMBOL_MAX 512

typedef struct {
    char name[LV_MAX_IDENTIFIER];
    char internal[LV_MAX_IDENTIFIER];
    char ctor_internal[LV_MAX_IDENTIFIER];
    int is_class;
    int exported;
} LvModuleSymbol;

typedef struct {
    char visible[LV_MAX_IDENTIFIER];
    char target[LV_MAX_IDENTIFIER];
} LvModuleLink;

typedef struct LvModuleExports {
    char path[LV_MODULE_PATH_MAX];

    /* Lista histórica de nomes exportados, mantida para compatibilidade
     * estrutural com o runtime 3.6 e para documentação/diagnóstico. */
    char (*names)[LV_MAX_IDENTIFIER];
    int count;
    int capacity;

    int module_id;
    int loaded;
    int has_explicit_exports;

    LvModuleSymbol symbols[LV_MODULE_SYMBOL_MAX];
    int symbol_count;

    LvModuleLink links[LV_MODULE_SYMBOL_MAX * 2];
    int link_count;

    struct LvModuleExports *next;
} LvModuleExports;

typedef struct {
    char loaded[LV_MODULE_MAX][LV_MODULE_PATH_MAX];
    int loaded_count;
    char loading[LV_MODULE_MAX][LV_MODULE_PATH_MAX];
    int loading_count;
    int next_module_id;
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