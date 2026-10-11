/* ============================================================================
 * LEVIATA 3.3 - EXECUÇÃO DE ARQUIVOS E MÓDULOS
 *
 * `usar "arquivo.pt"` é resolvido antes da compilação do arquivo importador.
 * Cada módulo é executado uma única vez por processo, caminhos são relativos
 * ao arquivo que contém o `usar`, e ciclos são detectados explicitamente.
 *
 * 3.3 usa namespace global compartilhado. Exportação/alias/isolamento ficam
 * para uma revisão futura, em vez de fingir namespaces por renomeação parcial.
 * ============================================================================ */
#include "leviata_runtime.h"
#include "leviata_ast.h"
#include "leviata_compiler.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

AstNode *leviata_parse(const char *source, int *had_error);

static char *runtime_read_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "[LEVIATA][Modulo] Nao foi possivel abrir: %s\n", path);
        return NULL;
    }

    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return NULL;
    }
    long size = ftell(f);
    if (size < 0) {
        fclose(f);
        return NULL;
    }
    rewind(f);

    char *buf = (char*)malloc((size_t)size + 1);
    if (!buf) {
        fclose(f);
        fprintf(stderr, "[LEVIATA][Modulo] Sem memoria para: %s\n", path);
        return NULL;
    }

    size_t count = fread(buf, 1, (size_t)size, f);
    buf[count] = '\0';
    fclose(f);
    return buf;
}

static int path_is_absolute(const char *path) {
    if (!path || !*path) return 0;
    if (path[0] == '/' || path[0] == '\\') return 1;
    return isalpha((unsigned char)path[0]) && path[1] == ':';
}

static void path_normalize(const char *input, char *out, size_t out_size) {
    if (!out || out_size == 0) return;
    out[0] = '\0';
    if (!input || !*input) {
        snprintf(out, out_size, ".");
        return;
    }

    char temp[LV_MODULE_PATH_MAX];
    size_t len = strlen(input);
    if (len >= sizeof(temp)) len = sizeof(temp) - 1;
    for (size_t i = 0; i < len; i++)
        temp[i] = input[i] == '\\' ? '/' : input[i];
    temp[len] = '\0';

    int has_drive = isalpha((unsigned char)temp[0]) && temp[1] == ':';
    int absolute = temp[0] == '/' || (has_drive && temp[2] == '/');
    char drive[3] = {0, 0, 0};
    char *cursor = temp;

    if (has_drive) {
        drive[0] = temp[0];
        drive[1] = ':';
        cursor = temp + 2;
        if (*cursor == '/') cursor++;
    } else if (*cursor == '/') {
        cursor++;
    }

    char *parts[256];
    int count = 0;
    char *tok = strtok(cursor, "/");
    while (tok && count < 256) {
        if (strcmp(tok, ".") == 0 || tok[0] == '\0') {
            /* ignora */
        } else if (strcmp(tok, "..") == 0) {
            if (count > 0 && strcmp(parts[count - 1], "..") != 0)
                count--;
            else if (!absolute)
                parts[count++] = tok;
        } else {
            parts[count++] = tok;
        }
        tok = strtok(NULL, "/");
    }

    size_t used = 0;
    if (has_drive) {
        int n = snprintf(out, out_size, "%s%s", drive, absolute ? "/" : "");
        used = n > 0 ? (size_t)n : 0;
    } else if (absolute) {
        snprintf(out, out_size, "/");
        used = 1;
    }

    for (int i = 0; i < count; i++) {
        if (used > 0 && out[used - 1] != '/') {
            if (used + 1 < out_size) {
                out[used++] = '/';
                out[used] = '\0';
            }
        }
        size_t part_len = strlen(parts[i]);
        if (used + part_len >= out_size)
            part_len = out_size - used - 1;
        memcpy(out + used, parts[i], part_len);
        used += part_len;
        out[used] = '\0';
    }

    if (used == 0)
        snprintf(out, out_size, absolute ? "/" : ".");
}

static void path_dirname(const char *origin, char *out, size_t out_size) {
    if (!origin || !*origin || origin[0] == '<') {
        snprintf(out, out_size, ".");
        return;
    }

    char normalized[LV_MODULE_PATH_MAX];
    path_normalize(origin, normalized, sizeof(normalized));
    char *last = strrchr(normalized, '/');
    if (!last) {
        snprintf(out, out_size, ".");
        return;
    }

    if (last == normalized) {
        snprintf(out, out_size, "/");
        return;
    }

    *last = '\0';
    snprintf(out, out_size, "%s", normalized);
}

static void resolve_import(const char *origin,
                           const char *requested,
                           char *out,
                           size_t out_size) {
    if (path_is_absolute(requested)) {
        path_normalize(requested, out, out_size);
        return;
    }

    char dir[LV_MODULE_PATH_MAX];
    char combined[LV_MODULE_PATH_MAX * 2];
    path_dirname(origin, dir, sizeof(dir));
    snprintf(combined, sizeof(combined), "%s/%s", dir, requested);
    path_normalize(combined, out, out_size);
}

static int path_in_list(char list[][LV_MODULE_PATH_MAX], int count,
                        const char *path) {
    for (int i = 0; i < count; i++) {
        if (strcmp(list[i], path) == 0) return 1;
    }
    return 0;
}

static int runtime_add_loaded(LvRuntime *rt, const char *path) {
    if (path_in_list(rt->loaded, rt->loaded_count, path)) return 1;
    if (rt->loaded_count >= LV_MODULE_MAX) {
        fprintf(stderr, "[LEVIATA][Modulo] Limite de modulos carregados atingido\n");
        return 0;
    }
    strncpy(rt->loaded[rt->loaded_count], path, LV_MODULE_PATH_MAX - 1);
    rt->loaded[rt->loaded_count][LV_MODULE_PATH_MAX - 1] = '\0';
    rt->loaded_count++;
    return 1;
}

static void print_cycle(LvRuntime *rt, const char *path) {
    fprintf(stderr, "[LEVIATA][Modulo] Ciclo de importacao detectado:\n");
    for (int i = 0; i < rt->loading_count; i++)
        fprintf(stderr, "  -> %s\n", rt->loading[i]);
    fprintf(stderr, "  -> %s\n", path);
}

static int ast_contains_nested_use(AstNode *node) {
    if (!node) return 0;
    if (node->type == AST_USE) return 1;

    switch (node->type) {
        case AST_BLOCK:
            for (int i = 0; i < node->as.block.count; i++)
                if (ast_contains_nested_use(node->as.block.stmts[i])) return 1;
            break;
        case AST_FUNC_DECL:
            return ast_contains_nested_use(node->as.func_decl.body);
        case AST_CLASS_DECL:
            for (int i = 0; i < node->as.class_decl.member_count; i++)
                if (ast_contains_nested_use(node->as.class_decl.members[i])) return 1;
            break;
        case AST_IF:
            return ast_contains_nested_use(node->as.if_stmt.then_branch) ||
                   ast_contains_nested_use(node->as.if_stmt.else_branch);
        case AST_WHILE:
            return ast_contains_nested_use(node->as.while_stmt.body);
        case AST_FOR_RANGE:
            return ast_contains_nested_use(node->as.for_range.body);
        case AST_FOR_EACH:
            return ast_contains_nested_use(node->as.for_each.body);
        default:
            break;
    }
    return 0;
}

static int runtime_load_module(LvRuntime *rt, LvVM *vm, const char *path);

static int process_imports(LvRuntime *rt,
                           LvVM *vm,
                           AstNode *program,
                           const char *origin) {
    if (!program || program->type != AST_PROGRAM) return 1;

    for (int i = 0; i < program->as.program.count; i++) {
        AstNode *stmt = program->as.program.stmts[i];
        if (!stmt) continue;

        if (stmt->type == AST_USE) {
            char resolved[LV_MODULE_PATH_MAX];
            resolve_import(origin, stmt->as.use_stmt.path,
                           resolved, sizeof(resolved));
            if (!runtime_load_module(rt, vm, resolved))
                return 0;

            /* O módulo já foi executado. Para o compilador, esta linha é um
             * statement sem efeito. O payload de AST_USE é inline. */
            stmt->type = AST_NULL_LIT;
            continue;
        }

        if (ast_contains_nested_use(stmt)) {
            fprintf(stderr,
                    "[LEVIATA][Modulo] 'usar' so pode aparecer no escopo principal (%s)\n",
                    origin ? origin : "<fonte>");
            return 0;
        }
    }

    return 1;
}

void leviata_runtime_init(LvRuntime *rt) {
    if (!rt) return;
    memset(rt, 0, sizeof(*rt));
}

int leviata_execute_source(LvRuntime *rt,
                           LvVM *vm,
                           const char *source,
                           const char *origin,
                           int show_ast) {
    if (!rt || !vm || !source) return 0;

    int parse_error = 0;
    AstNode *ast = leviata_parse(source, &parse_error);
    if (!ast || parse_error) {
        fprintf(stderr, "[LEVIATA] Erros de parse em %s. Execucao cancelada.\n",
                origin ? origin : "<fonte>");
        ast_free(ast);
        return 0;
    }

    if (show_ast) {
        printf("\n=== AST GERADA ===\n");
        ast_print(ast, 0);
        printf("==================\n\n");
    }

    if (!process_imports(rt, vm, ast, origin)) {
        ast_free(ast);
        return 0;
    }

    Compiler compiler;
    compiler_init(&compiler, vm, NULL);
    Chunk chunk;
    int ok = compiler_compile_program(&compiler, ast, &chunk);
    ast_free(ast);
    compiler_free(&compiler);

    if (!ok) {
        fprintf(stderr, "[LEVIATA] Erros de compilacao em %s.\n",
                origin ? origin : "<fonte>");
        chunk_free(&chunk);
        return 0;
    }

    vm->had_error = 0;
    int result = vm_run_chunk(vm, &chunk);
    chunk_free(&chunk);
    return result;
}

static int execute_file_internal(LvRuntime *rt,
                                 LvVM *vm,
                                 const char *path,
                                 int show_ast,
                                 int skip_if_loaded) {
    char normalized[LV_MODULE_PATH_MAX];
    path_normalize(path, normalized, sizeof(normalized));

    if (skip_if_loaded &&
        path_in_list(rt->loaded, rt->loaded_count, normalized))
        return 1;

    if (path_in_list(rt->loading, rt->loading_count, normalized)) {
        print_cycle(rt, normalized);
        return 0;
    }

    if (rt->loading_count >= LV_MODULE_MAX) {
        fprintf(stderr, "[LEVIATA][Modulo] Profundidade de modulos excedida\n");
        return 0;
    }

    strncpy(rt->loading[rt->loading_count], normalized, LV_MODULE_PATH_MAX - 1);
    rt->loading[rt->loading_count][LV_MODULE_PATH_MAX - 1] = '\0';
    rt->loading_count++;

    char *source = runtime_read_file(normalized);
    int ok = 0;
    if (source) {
        ok = leviata_execute_source(rt, vm, source, normalized, show_ast);
        free(source);
    }

    rt->loading_count--;
    if (ok && !runtime_add_loaded(rt, normalized))
        ok = 0;
    return ok;
}

static int runtime_load_module(LvRuntime *rt, LvVM *vm, const char *path) {
    return execute_file_internal(rt, vm, path, 0, 1);
}

int leviata_execute_file(LvRuntime *rt,
                         LvVM *vm,
                         const char *path,
                         int show_ast) {
    if (!rt || !vm || !path) return 0;
    return execute_file_internal(rt, vm, path, show_ast, 0);
}