/* ============================================================================
 * LEVIATA 3.7 - RUNTIME DE MODULOS ISOLADOS
 *
 * O linker troca referências globais próprias do módulo por nomes internos
 * inacessíveis ao fonte (@mN:X). Imports entre módulos apontam diretamente
 * para esses símbolos. Para chamadas qualificadas dentro de outro módulo,
 * nomes como `util.funcao` são instalados apenas durante a compilação como
 * dicas de resolução estática e removidos antes da execução.
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
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return NULL; }
    long size = ftell(f);
    if (size < 0) { fclose(f); return NULL; }
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
    for (int i = 0; i < count; i++)
        if (strcmp(list[i], path) == 0) return 1;
    return 0;
}

static int runtime_add_loaded(LvRuntime *rt, const char *path) {
    if (path_in_list(rt->loaded, rt->loaded_count, path)) return 1;
    if (rt->loaded_count >= LV_MODULE_MAX) {
        fprintf(stderr, "[LEVIATA][Modulo] Limite de modulos carregados atingido\n");
        return 0;
    }
    snprintf(rt->loaded[rt->loaded_count], LV_MODULE_PATH_MAX, "%s", path);
    rt->loaded_count++;
    return 1;
}

static void print_cycle(LvRuntime *rt, const char *path) {
    fprintf(stderr, "[LEVIATA][Modulo] Ciclo de importacao detectado:\n");
    for (int i = 0; i < rt->loading_count; i++)
        fprintf(stderr, "  -> %s\n", rt->loading[i]);
    fprintf(stderr, "  -> %s\n", path);
}

static LvModuleExports *module_find(LvRuntime *rt, const char *path, int create) {
    if (!rt || !path) return NULL;
    char normalized[LV_MODULE_PATH_MAX];
    if (path[0] == '<') snprintf(normalized, sizeof(normalized), "%s", path);
    else path_normalize(path, normalized, sizeof(normalized));

    for (LvModuleExports *m = rt->exports; m; m = m->next)
        if (strcmp(m->path, normalized) == 0) return m;
    if (!create) return NULL;

    LvModuleExports *m = (LvModuleExports*)calloc(1, sizeof(LvModuleExports));
    if (!m) return NULL;
    snprintf(m->path, sizeof(m->path), "%s", normalized);
    m->capacity = 8;
    m->names = (char(*)[LV_MAX_IDENTIFIER])calloc(
        (size_t)m->capacity, LV_MAX_IDENTIFIER);
    if (!m->names) { free(m); return NULL; }
    m->module_id = ++rt->next_module_id;
    m->next = rt->exports;
    rt->exports = m;
    return m;
}

static LvModuleSymbol *module_symbol(LvModuleExports *m, const char *name) {
    if (!m || !name) return NULL;
    for (int i = 0; i < m->symbol_count; i++)
        if (strcmp(m->symbols[i].name, name) == 0) return &m->symbols[i];
    return NULL;
}

static int export_name_add(LvModuleExports *m, const char *name) {
    if (!m || !name) return 0;
    for (int i = 0; i < m->count; i++)
        if (strcmp(m->names[i], name) == 0) return 1;

    if (m->count >= m->capacity) {
        int nova = m->capacity * 2;
        char (*p)[LV_MAX_IDENTIFIER] = (char(*)[LV_MAX_IDENTIFIER])realloc(
            m->names, (size_t)nova * LV_MAX_IDENTIFIER);
        if (!p) return 0;
        memset(p + m->capacity, 0,
               (size_t)(nova - m->capacity) * LV_MAX_IDENTIFIER);
        m->names = p;
        m->capacity = nova;
    }
    snprintf(m->names[m->count], LV_MAX_IDENTIFIER, "%s", name);
    m->count++;
    return 1;
}

static int module_symbol_add(LvModuleExports *m,
                             const char *name,
                             int is_class) {
    if (!m || !name) return 0;
    if (module_symbol(m, name)) return 1;
    if (m->symbol_count >= LV_MODULE_SYMBOL_MAX) {
        fprintf(stderr, "[LEVIATA][Modulo] Muitos simbolos em %s\n", m->path);
        return 0;
    }

    int index = m->symbol_count;
    LvModuleSymbol *s = &m->symbols[m->symbol_count++];
    snprintf(s->name, sizeof(s->name), "%s", name);
    snprintf(s->internal, sizeof(s->internal), "@m%d:%d", m->module_id, index);
    s->is_class = is_class;
    if (is_class)
        snprintf(s->ctor_internal, sizeof(s->ctor_internal),
                 "@m%d:c%d", m->module_id, index);
    return 1;
}

static int call_is_marker(AstNode *node, const char *name) {
    return node && node->type == AST_CALL && node->as.call.callee &&
           node->as.call.callee->type == AST_IDENT &&
           strcmp(node->as.call.callee->as.ident.name, name) == 0;
}

static int collect_module_symbols(LvModuleExports *m, AstNode *program) {
    if (!m || !program || program->type != AST_PROGRAM) return 0;
    m->symbol_count = 0;
    m->link_count = 0;
    m->count = 0;
    m->has_explicit_exports = 0;

    for (int i = 0; i < program->as.program.count; i++) {
        AstNode *stmt = program->as.program.stmts[i];
        if (!stmt) continue;
        if (stmt->type == AST_VAR_DECL || stmt->type == AST_CONST_DECL) {
            if (!module_symbol_add(m, stmt->as.var_decl.name, 0)) return 0;
        } else if (stmt->type == AST_FUNC_DECL) {
            if (!module_symbol_add(m, stmt->as.func_decl.name, 0)) return 0;
        } else if (stmt->type == AST_CLASS_DECL) {
            if (!module_symbol_add(m, stmt->as.class_decl.name, 1)) return 0;
        }
    }

    for (int i = 0; i < program->as.program.count; i++) {
        AstNode *stmt = program->as.program.stmts[i];
        if (!call_is_marker(stmt, "__leviata_exportar")) continue;
        if (stmt->as.call.argc != 1 || !stmt->as.call.args[0] ||
            stmt->as.call.args[0]->type != AST_STR_LIT) {
            fprintf(stderr, "[LEVIATA][Modulo] Exportacao interna invalida em %s\n", m->path);
            return 0;
        }
        const char *name = stmt->as.call.args[0]->as.str_lit.value;
        LvModuleSymbol *s = module_symbol(m, name);
        if (!s) {
            fprintf(stderr,
                    "[LEVIATA][Modulo] '%s' foi exportado, mas nao foi declarado em %s\n",
                    name, m->path);
            return 0;
        }
        m->has_explicit_exports = 1;
        s->exported = 1;
        if (!export_name_add(m, name)) return 0;
    }

    /* Compatibilidade 3.3-3.6: arquivo sem `exportar` publica suas declarações
     * quando importado. Os nomes continuam internamente isolados. */
    if (!m->has_explicit_exports) {
        for (int i = 0; i < m->symbol_count; i++) {
            m->symbols[i].exported = 1;
            if (!export_name_add(m, m->symbols[i].name)) return 0;
        }
    }
    return 1;
}

static int module_link_add(LvModuleExports *m,
                           const char *visible,
                           const char *target) {
    if (!m || !visible || !target) return 0;
    if (module_symbol(m, visible)) {
        fprintf(stderr,
                "[LEVIATA][Modulo] Import '%s' colide com declaracao local em %s\n",
                visible, m->path);
        return 0;
    }
    for (int i = 0; i < m->link_count; i++) {
        if (strcmp(m->links[i].visible, visible) == 0) {
            if (strcmp(m->links[i].target, target) == 0) return 1;
            fprintf(stderr,
                    "[LEVIATA][Modulo] Import ambiguo '%s' em %s\n",
                    visible, m->path);
            return 0;
        }
    }
    if (m->link_count >= LV_MODULE_SYMBOL_MAX * 2) {
        fprintf(stderr, "[LEVIATA][Modulo] Muitos imports em %s\n", m->path);
        return 0;
    }
    LvModuleLink *l = &m->links[m->link_count++];
    snprintf(l->visible, sizeof(l->visible), "%s", visible);
    snprintf(l->target, sizeof(l->target), "%s", target);
    return 1;
}

static int link_exports_into_module(LvModuleExports *dst,
                                    LvModuleExports *src,
                                    const char *alias) {
    if (!dst || !src) return 0;
    for (int i = 0; i < src->symbol_count; i++) {
        LvModuleSymbol *s = &src->symbols[i];
        if (!s->exported) continue;

        char visible[LV_MAX_IDENTIFIER];
        if (alias && *alias) {
            int n = snprintf(visible, sizeof(visible), "%s.%s", alias, s->name);
            if (n < 0 || (size_t)n >= sizeof(visible)) {
                fprintf(stderr, "[LEVIATA][Modulo] Nome importado longo demais\n");
                return 0;
            }
        } else {
            snprintf(visible, sizeof(visible), "%s", s->name);
        }
        if (!module_link_add(dst, visible, s->internal)) return 0;

        if (s->is_class) {
            char ctor_visible[LV_MAX_IDENTIFIER];
            int n = snprintf(ctor_visible, sizeof(ctor_visible), "novo %s", visible);
            if (n < 0 || (size_t)n >= sizeof(ctor_visible)) return 0;
            if (!module_link_add(dst, ctor_visible, s->ctor_internal)) return 0;
        }
    }
    return 1;
}

static int bind_root_symbol(LvVM *vm,
                            const char *visible,
                            const char *internal) {
    LvObject *value = hmap_get(&vm->globals, internal);
    if (!value) {
        fprintf(stderr,
                "[LEVIATA][Modulo] Simbolo interno '%s' nao existe apos carregar modulo\n",
                internal);
        return 0;
    }
    LvObject *existing = hmap_get(&vm->globals, visible);
    if (existing && existing != value) {
        fprintf(stderr, "[LEVIATA][Modulo] Colisao ao importar '%s'\n", visible);
        return 0;
    }
    hmap_set(&vm->globals, visible, value);
    return 1;
}

static int bind_exports_into_root(LvVM *vm,
                                  LvModuleExports *src,
                                  const char *alias) {
    if (!vm || !src) return 0;
    for (int i = 0; i < src->symbol_count; i++) {
        LvModuleSymbol *s = &src->symbols[i];
        if (!s->exported) continue;

        char visible[LV_MAX_IDENTIFIER];
        if (alias && *alias) {
            int n = snprintf(visible, sizeof(visible), "%s.%s", alias, s->name);
            if (n < 0 || (size_t)n >= sizeof(visible)) return 0;
        } else {
            snprintf(visible, sizeof(visible), "%s", s->name);
        }
        if (!bind_root_symbol(vm, visible, s->internal)) return 0;

        if (s->is_class) {
            char ctor_visible[LV_MAX_IDENTIFIER];
            int n = snprintf(ctor_visible, sizeof(ctor_visible), "novo %s", visible);
            if (n < 0 || (size_t)n >= sizeof(ctor_visible)) return 0;
            if (!bind_root_symbol(vm, ctor_visible, s->ctor_internal)) return 0;
        }
    }
    return 1;
}

static const char *module_resolve_name(LvModuleExports *m, const char *name) {
    if (!m || !name) return NULL;
    for (int i = 0; i < m->symbol_count; i++) {
        LvModuleSymbol *s = &m->symbols[i];
        if (strcmp(s->name, name) == 0) return s->internal;
        if (s->is_class) {
            char ctor[LV_MAX_IDENTIFIER];
            int n = snprintf(ctor, sizeof(ctor), "novo %s", s->name);
            if (n > 0 && (size_t)n < sizeof(ctor) && strcmp(ctor, name) == 0)
                return s->ctor_internal;
        }
    }
    for (int i = 0; i < m->link_count; i++)
        if (strcmp(m->links[i].visible, name) == 0)
            return m->links[i].target;
    return NULL;
}

static void rewrite_chunk_globals(Chunk *chunk, LvModuleExports *m) {
    if (!chunk || !m) return;
    for (int i = 0; i < chunk->count; i++) {
        Instruction *inst = &chunk->code[i];
        if (inst->opcode != OP_LOAD_GLOBAL && inst->opcode != OP_STORE_GLOBAL)
            continue;
        int ci = inst->operand;
        if (ci < 0 || ci >= chunk->const_count) continue;
        ConstValue *cv = &chunk->constants[ci];
        if (cv->type != LV_TYPE_STRING) continue;
        const char *target = module_resolve_name(m, cv->as.s_val);
        if (target) snprintf(cv->as.s_val, sizeof(cv->as.s_val), "%s", target);
    }
}

static void rewrite_module_bytecode(LvVM *vm,
                                    LvModuleExports *m,
                                    Chunk *main_chunk,
                                    int function_start) {
    rewrite_chunk_globals(main_chunk, m);
    for (int i = function_start; i < vm->func_count; i++)
        rewrite_chunk_globals(vm->functions[i], m);
}

/* O compilador reconhece API qualificada consultando vm->globals. Links de
 * módulos não devem permanecer ali, então são inseridos somente enquanto o
 * AST atual é compilado e removidos por reconstrução segura da HashMap. */
static int hint_name_exists(char names[][LV_MAX_IDENTIFIER], int count,
                            const char *name) {
    for (int i = 0; i < count; i++)
        if (strcmp(names[i], name) == 0) return 1;
    return 0;
}

static int install_compile_hints(LvVM *vm,
                                 LvModuleExports *m,
                                 char added[][LV_MAX_IDENTIFIER],
                                 int *added_count) {
    *added_count = 0;
    if (!vm || !m) return 1;

    for (int i = 0; i < m->link_count; i++) {
        const char *visible = m->links[i].visible;
        if (!strchr(visible, '.')) continue;
        if (strncmp(visible, "novo ", 5) == 0) continue;
        if (hmap_has(&vm->globals, visible)) continue;
        if (*added_count >= LV_MODULE_SYMBOL_MAX * 2) return 0;

        LvObject *value = hmap_get(&vm->globals, m->links[i].target);
        if (!value) value = vm->nil_singleton;
        hmap_set(&vm->globals, visible, value);
        snprintf(added[*added_count], LV_MAX_IDENTIFIER, "%s", visible);
        (*added_count)++;
    }
    return 1;
}

static int remove_compile_hints(LvVM *vm,
                                char added[][LV_MAX_IDENTIFIER],
                                int added_count) {
    if (!vm || added_count <= 0) return 1;

    HashMap rebuilt;
    hmap_init(&rebuilt, vm->globals.capacity);
    for (int i = 0; i < vm->globals.capacity; i++) {
        HMapEntry *e = &vm->globals.entries[i];
        if (!e->in_use) continue;
        if (hint_name_exists(added, added_count, e->key)) continue;
        hmap_set(&rebuilt, e->key, e->value);
    }
    free(vm->globals.entries);
    vm->globals = rebuilt;
    return 1;
}

static int ast_contains_module_directive(AstNode *node) {
    if (!node) return 0;
    if (node->type == AST_USE ||
        call_is_marker(node, "__leviata_exportar") ||
        call_is_marker(node, "__leviata_alias_import"))
        return 1;

    switch (node->type) {
        case AST_BLOCK:
            for (int i = 0; i < node->as.block.count; i++)
                if (ast_contains_module_directive(node->as.block.stmts[i])) return 1;
            break;
        case AST_FUNC_DECL:
            return ast_contains_module_directive(node->as.func_decl.body);
        case AST_CLASS_DECL:
            for (int i = 0; i < node->as.class_decl.member_count; i++)
                if (ast_contains_module_directive(node->as.class_decl.members[i])) return 1;
            break;
        case AST_IF:
            return ast_contains_module_directive(node->as.if_stmt.then_branch) ||
                   ast_contains_module_directive(node->as.if_stmt.else_branch);
        case AST_WHILE:
            return ast_contains_module_directive(node->as.while_stmt.body);
        case AST_FOR_RANGE:
            return ast_contains_module_directive(node->as.for_range.body);
        case AST_FOR_EACH:
            return ast_contains_module_directive(node->as.for_each.body);
        default:
            break;
    }
    return 0;
}

static void consume_program_stmt(AstNode *program, int index) {
    if (!program || program->type != AST_PROGRAM ||
        index < 0 || index >= program->as.program.count)
        return;
    AstNode *old = program->as.program.stmts[index];
    int line = old ? old->line : 1;
    program->as.program.stmts[index] = ast_alloc(AST_NULL_LIT, line);
    ast_free(old);
}

static int alias_marker_for_use(AstNode *program, int index, const char *path) {
    if (!program || index + 1 >= program->as.program.count) return 0;
    AstNode *next = program->as.program.stmts[index + 1];
    if (!call_is_marker(next, "__leviata_alias_import") ||
        next->as.call.argc != 2 || !next->as.call.args[0] ||
        next->as.call.args[0]->type != AST_STR_LIT)
        return 0;
    return strcmp(next->as.call.args[0]->as.str_lit.value, path) == 0;
}

static LvModuleExports *runtime_load_module(LvRuntime *rt,
                                            LvVM *vm,
                                            const char *path);

static int process_imports(LvRuntime *rt,
                           LvVM *vm,
                           AstNode *program,
                           const char *origin,
                           LvModuleExports *current_module) {
    if (!program || program->type != AST_PROGRAM) return 1;

    for (int i = 0; i < program->as.program.count; i++) {
        AstNode *stmt = program->as.program.stmts[i];
        if (!stmt) continue;

        if (stmt->type == AST_USE) {
            char requested[LV_MODULE_PATH_MAX];
            size_t rlen = strlen(stmt->as.use_stmt.path);
            if (rlen >= sizeof(requested)) {
                fprintf(stderr, "[LEVIATA][Modulo] Caminho de importacao longo demais\n");
                return 0;
            }
            memcpy(requested, stmt->as.use_stmt.path, rlen + 1);
            int has_alias = alias_marker_for_use(program, i, requested);

            char resolved[LV_MODULE_PATH_MAX];
            resolve_import(origin, requested, resolved, sizeof(resolved));
            LvModuleExports *imported = runtime_load_module(rt, vm, resolved);
            if (!imported) return 0;

            if (!has_alias) {
                int ok = current_module
                    ? link_exports_into_module(current_module, imported, NULL)
                    : bind_exports_into_root(vm, imported, NULL);
                if (!ok) return 0;
            }
            consume_program_stmt(program, i);
            continue;
        }

        if (call_is_marker(stmt, "__leviata_exportar")) {
            consume_program_stmt(program, i);
            continue;
        }

        if (call_is_marker(stmt, "__leviata_alias_import")) {
            if (stmt->as.call.argc != 2 || !stmt->as.call.args[0] ||
                stmt->as.call.args[0]->type != AST_STR_LIT ||
                !stmt->as.call.args[1] ||
                stmt->as.call.args[1]->type != AST_STR_LIT) {
                fprintf(stderr, "[LEVIATA][Modulo] Marcador interno de alias invalido\n");
                return 0;
            }

            char resolved[LV_MODULE_PATH_MAX];
            resolve_import(origin,
                           stmt->as.call.args[0]->as.str_lit.value,
                           resolved, sizeof(resolved));
            LvModuleExports *imported = runtime_load_module(rt, vm, resolved);
            if (!imported) return 0;

            const char *alias = stmt->as.call.args[1]->as.str_lit.value;
            int ok = current_module
                ? link_exports_into_module(current_module, imported, alias)
                : bind_exports_into_root(vm, imported, alias);
            if (!ok) return 0;
            consume_program_stmt(program, i);
            continue;
        }

        if (ast_contains_module_directive(stmt)) {
            fprintf(stderr,
                    "[LEVIATA][Modulo] 'usar' e 'exportar' so podem aparecer no escopo principal (%s)\n",
                    origin ? origin : "<fonte>");
            return 0;
        }
    }
    return 1;
}

static int execute_source_internal(LvRuntime *rt,
                                   LvVM *vm,
                                   const char *source,
                                   const char *origin,
                                   int show_ast,
                                   LvModuleExports *module) {
    if (!rt || !vm || !source) return 0;

    int parse_error = 0;
    AstNode *ast = leviata_parse(source, &parse_error);
    if (!ast || parse_error) {
        fprintf(stderr, "[LEVIATA] Erros de parse em %s. Execucao cancelada.\n",
                origin ? origin : "<fonte>");
        ast_free(ast);
        return 0;
    }

    if (module && !collect_module_symbols(module, ast)) {
        ast_free(ast);
        return 0;
    }

    if (show_ast) {
        printf("\n=== AST GERADA ===\n");
        ast_print(ast, 0);
        printf("==================\n\n");
    }

    if (!process_imports(rt, vm, ast, origin, module)) {
        ast_free(ast);
        return 0;
    }

    char compile_hints[LV_MODULE_SYMBOL_MAX * 2][LV_MAX_IDENTIFIER];
    int compile_hint_count = 0;
    if (module &&
        !install_compile_hints(vm, module, compile_hints, &compile_hint_count)) {
        ast_free(ast);
        return 0;
    }

    int function_start = vm->func_count;
    Compiler compiler;
    compiler_init(&compiler, vm, NULL);
    Chunk chunk;
    int ok = compiler_compile_program(&compiler, ast, &chunk);
    ast_free(ast);
    compiler_free(&compiler);

    if (module)
        remove_compile_hints(vm, compile_hints, compile_hint_count);

    if (!ok) {
        fprintf(stderr, "[LEVIATA] Erros de compilacao em %s.\n",
                origin ? origin : "<fonte>");
        chunk_free(&chunk);
        return 0;
    }

    if (module)
        rewrite_module_bytecode(vm, module, &chunk, function_start);

    vm->had_error = 0;
    int result = vm_run_chunk(vm, &chunk);
    chunk_free(&chunk);
    return result;
}

static LvModuleExports *runtime_load_module(LvRuntime *rt,
                                            LvVM *vm,
                                            const char *path) {
    char normalized[LV_MODULE_PATH_MAX];
    path_normalize(path, normalized, sizeof(normalized));

    LvModuleExports *module = module_find(rt, normalized, 1);
    if (!module) return NULL;
    if (module->loaded) return module;

    if (path_in_list(rt->loading, rt->loading_count, normalized)) {
        print_cycle(rt, normalized);
        return NULL;
    }
    if (rt->loading_count >= LV_MODULE_MAX) {
        fprintf(stderr, "[LEVIATA][Modulo] Profundidade de modulos excedida\n");
        return NULL;
    }

    snprintf(rt->loading[rt->loading_count], LV_MODULE_PATH_MAX,
             "%s", normalized);
    rt->loading_count++;

    char *source = runtime_read_file(normalized);
    int ok = 0;
    if (source) {
        ok = execute_source_internal(rt, vm, source, normalized, 0, module);
        free(source);
    }

    rt->loading_count--;
    if (!ok) return NULL;
    module->loaded = 1;
    if (!runtime_add_loaded(rt, normalized)) return NULL;
    return module;
}

void leviata_runtime_init(LvRuntime *rt) {
    if (!rt) return;
    memset(rt, 0, sizeof(*rt));
}

void leviata_runtime_free(LvRuntime *rt) {
    if (!rt) return;
    LvModuleExports *m = rt->exports;
    while (m) {
        LvModuleExports *next = m->next;
        free(m->names);
        free(m);
        m = next;
    }
    rt->exports = NULL;
}

int leviata_execute_source(LvRuntime *rt,
                           LvVM *vm,
                           const char *source,
                           const char *origin,
                           int show_ast) {
    return execute_source_internal(rt, vm, source, origin, show_ast, NULL);
}

int leviata_execute_file(LvRuntime *rt,
                         LvVM *vm,
                         const char *path,
                         int show_ast) {
    if (!rt || !vm || !path) return 0;

    char normalized[LV_MODULE_PATH_MAX];
    path_normalize(path, normalized, sizeof(normalized));
    if (path_in_list(rt->loading, rt->loading_count, normalized)) {
        print_cycle(rt, normalized);
        return 0;
    }
    if (rt->loading_count >= LV_MODULE_MAX) return 0;

    snprintf(rt->loading[rt->loading_count], LV_MODULE_PATH_MAX,
             "%s", normalized);
    rt->loading_count++;

    char *source = runtime_read_file(normalized);
    int ok = 0;
    if (source) {
        ok = execute_source_internal(rt, vm, source, normalized, show_ast, NULL);
        free(source);
    }

    rt->loading_count--;
    return ok;
}
