/* ============================================================================
 * MOTOR LEVIATA - IMPLEMENTAÇÃO DA VM, GC E HASHMAP
 * ============================================================================ */
#include "leviata_vm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

/* ============================================================
 * HASHMAP
 * ============================================================ */
static uint32_t hmap_hash(const char *key) {
    uint32_t h = 2166136261u;
    while (*key) {
        h ^= (uint8_t)*key++;
        h *= 16777619u;
    }
    return h;
}

void hmap_init(HashMap *m, int capacity) {
    m->capacity = capacity;
    m->count    = 0;
    m->entries  = (HMapEntry*)calloc(capacity, sizeof(HMapEntry));
    if (!m->entries) {
        fprintf(stderr, "[LEVIATA] Falha ao alocar HashMap\n");
        exit(EXIT_FAILURE);
    }
}

void hmap_free(HashMap *m) {
    free(m->entries);
    m->entries  = NULL;
    m->count    = 0;
    m->capacity = 0;
}

void hmap_set(HashMap *m, const char *key, LvObject *value) {
    uint32_t idx = hmap_hash(key) % (uint32_t)m->capacity;
    for (int i = 0; i < m->capacity; i++) {
        uint32_t slot = (idx + i) % (uint32_t)m->capacity;
        HMapEntry *e = &m->entries[slot];
        if (!e->in_use || strcmp(e->key, key) == 0) {
            if (!e->in_use) m->count++;
            e->in_use = 1;
            strncpy(e->key, key, LV_MAX_IDENTIFIER - 1);
            e->key[LV_MAX_IDENTIFIER - 1] = '\0';
            e->value = value;
            return;
        }
    }
    fprintf(stderr, "[LEVIATA] HashMap cheio!\n");
    exit(EXIT_FAILURE);
}

LvObject *hmap_get(HashMap *m, const char *key) {
    uint32_t idx = hmap_hash(key) % (uint32_t)m->capacity;
    for (int i = 0; i < m->capacity; i++) {
        uint32_t slot = (idx + i) % (uint32_t)m->capacity;
        HMapEntry *e = &m->entries[slot];
        if (!e->in_use) return NULL;
        if (strcmp(e->key, key) == 0) return e->value;
    }
    return NULL;
}

int hmap_has(HashMap *m, const char *key) {
    return hmap_get(m, key) != NULL;
}

void hmap_delete(HashMap *m, const char *key) {
    uint32_t idx = hmap_hash(key) % (uint32_t)m->capacity;
    for (int i = 0; i < m->capacity; i++) {
        uint32_t slot = (idx + i) % (uint32_t)m->capacity;
        HMapEntry *e = &m->entries[slot];
        if (!e->in_use) return;
        if (strcmp(e->key, key) == 0) {
            e->in_use = 0;
            m->count--;
            return;
        }
    }
}

/* ============================================================
 * CHUNK (BYTECODE BUFFER)
 * ============================================================ */
void chunk_init(Chunk *c) {
    c->code          = (Instruction*)malloc(256 * sizeof(Instruction));
    c->count         = 0;
    c->capacity      = 256;
    c->constants     = (ConstValue*)malloc(64 * sizeof(ConstValue));
    c->const_count   = 0;
    c->const_capacity = 64;
    c->lines         = (int*)malloc(256 * sizeof(int));
}

void chunk_free(Chunk *c) {
    free(c->code);
    free(c->constants);
    free(c->lines);
    c->code          = NULL;
    c->constants     = NULL;
    c->lines         = NULL;
    c->count         = 0;
    c->const_count   = 0;
}

void chunk_write(Chunk *c, uint8_t opcode, int32_t operand, int line) {
    if (c->count >= c->capacity) {
        c->capacity *= 2;
        c->code  = (Instruction*)realloc(c->code,  c->capacity * sizeof(Instruction));
        c->lines = (int*)realloc(c->lines, c->capacity * sizeof(int));
    }
    c->code[c->count].opcode  = opcode;
    c->code[c->count].operand = operand;
    c->lines[c->count]        = line;
    c->count++;
}

int chunk_add_constant(Chunk *c, ConstValue cv) {
    if (c->const_count >= c->const_capacity) {
        c->const_capacity *= 2;
        c->constants = (ConstValue*)realloc(c->constants, c->const_capacity * sizeof(ConstValue));
    }
    c->constants[c->const_count++] = cv;
    return c->const_count - 1;
}

/* ============================================================
 * GARBAGE COLLECTOR - MARK-AND-SWEEP
 * ============================================================ */
LvObject *gc_alloc_object(LvVM *vm, LvType type) {
    LvObject *obj = (LvObject*)calloc(1, sizeof(LvObject));
    if (!obj) {
        fprintf(stderr, "[LEVIATA][GC] Sem memória!\n");
        exit(EXIT_FAILURE);
    }
    obj->type         = type;
    obj->gc_marked    = 0;
    obj->gc_next      = vm->gc.all_objects;
    vm->gc.all_objects = obj;
    vm->gc.object_count++;
    vm->gc.bytes_allocated += sizeof(LvObject);

    if (vm->gc.object_count >= vm->gc.gc_threshold) {
        gc_collect(vm);
    }

    return obj;
}

LvObject *gc_alloc_int(LvVM *vm, int64_t value) {
    LvObject *o = gc_alloc_object(vm, LV_TYPE_INT);
    o->as.i_val = value;
    return o;
}

LvObject *gc_alloc_float(LvVM *vm, double value) {
    LvObject *o = gc_alloc_object(vm, LV_TYPE_FLOAT);
    o->as.f_val = value;
    return o;
}

LvObject *gc_alloc_bool(LvVM *vm, int value) {
    return value ? vm->true_singleton : vm->false_singleton;
}

LvObject *gc_alloc_string(LvVM *vm, const char *str, size_t len) {
    LvObject *o = gc_alloc_object(vm, LV_TYPE_STRING);
    o->as.string.data   = (char*)malloc(len + 1);
    o->as.string.length = len;
    memcpy(o->as.string.data, str, len);
    o->as.string.data[len] = '\0';
    vm->gc.bytes_allocated += len + 1;
    return o;
}

LvObject *gc_alloc_list(LvVM *vm) {
    LvObject *o = gc_alloc_object(vm, LV_TYPE_LIST);
    o->as.list.items    = (LvObject**)malloc(8 * sizeof(LvObject*));
    o->as.list.count    = 0;
    o->as.list.capacity = 8;
    return o;
}

/*
 * gc_alloc_function — cria um objeto de função que aponta para um Chunk
 * já compilado e registrado em vm->functions[].
 * O Chunk é de propriedade da tabela vm->functions[], NÃO do GC.
 * O GC gerencia apenas o invólucro LvObject.
 */
LvObject *gc_alloc_function(LvVM *vm, Chunk *chunk, const char *name, int arity) {
    LvObject *o = gc_alloc_object(vm, LV_TYPE_FUNCTION);
    o->as.function.chunk = chunk;
    o->as.function.arity = arity;
    o->as.function.upvalue_count = 0;
    strncpy(o->as.function.name, name, LV_MAX_IDENTIFIER - 1);
    o->as.function.name[LV_MAX_IDENTIFIER - 1] = '\0';
    return o;
}

void gc_mark_object(LvObject *obj) {
    if (!obj || obj->gc_marked) return;
    obj->gc_marked = 1;

    switch (obj->type) {
        case LV_TYPE_LIST:
            for (int i = 0; i < obj->as.list.count; i++)
                gc_mark_object(obj->as.list.items[i]);
            break;
        case LV_TYPE_MAP:
            for (int i = 0; i < obj->as.map.capacity; i++) {
                if (obj->as.map.buckets[i].occupied)
                    gc_mark_object(obj->as.map.buckets[i].value);
            }
            break;
        case LV_TYPE_FUNCTION:
            /* O Chunk usa ConstValue (tipo-por-valor), sem sub-objetos GC.
             * Marcar o LvObject invólucro é suficiente. */
            break;
        default:
            break;
    }
}

static void gc_mark_roots(LvVM *vm) {
    /* Marca stack de valores */
    for (int i = 0; i < vm->stack_top; i++)
        gc_mark_object(vm->stack[i]);

    /* Marca variáveis globais */
    for (int i = 0; i < vm->globals.capacity; i++) {
        if (vm->globals.entries[i].in_use)
            gc_mark_object(vm->globals.entries[i].value);
    }

    /* Marca singletons */
    gc_mark_object(vm->nil_singleton);
    gc_mark_object(vm->true_singleton);
    gc_mark_object(vm->false_singleton);
}

void gc_sweep(LvVM *vm) {
    LvObject **cur = &vm->gc.all_objects;
    while (*cur) {
        LvObject *obj = *cur;
        if (!obj->gc_marked) {
            *cur = obj->gc_next;
            vm->gc.object_count--;
            vm->gc.bytes_allocated -= sizeof(LvObject);

            /* Libera dados alocados internamente */
            if (obj->type == LV_TYPE_STRING && obj->as.string.data) {
                vm->gc.bytes_allocated -= obj->as.string.length + 1;
                free(obj->as.string.data);
            } else if (obj->type == LV_TYPE_LIST && obj->as.list.items) {
                free(obj->as.list.items);
            } else if (obj->type == LV_TYPE_FUNCTION) {
                /* O Chunk pertence a vm->functions[] — não liberar aqui.
                 * Apenas o invólucro LvObject é coletado pelo GC. */
            }

            free(obj);
        } else {
            obj->gc_marked = 0; /* reset para próximo ciclo */
            cur = &obj->gc_next;
        }
    }
}

void gc_collect(LvVM *vm) {
    gc_mark_roots(vm);
    gc_sweep(vm);
    vm->gc.gc_threshold = vm->gc.object_count * 2;
    if (vm->gc.gc_threshold < LV_GC_THRESHOLD)
        vm->gc.gc_threshold = LV_GC_THRESHOLD;
}

/* ============================================================
 * VM - STACK
 * ============================================================ */
void vm_push(LvVM *vm, LvObject *obj) {
    if (vm->stack_top >= LV_STACK_MAX) {
        fprintf(stderr, "[LEVIATA][VM] Stack overflow!\n");
        exit(EXIT_FAILURE);
    }
    vm->stack[vm->stack_top++] = obj;
}

LvObject *vm_pop(LvVM *vm) {
    if (vm->stack_top <= 0) {
        fprintf(stderr, "[LEVIATA][VM] Stack underflow!\n");
        exit(EXIT_FAILURE);
    }
    return vm->stack[--vm->stack_top];
}

LvObject *vm_peek(LvVM *vm, int distance) {
    if (vm->stack_top - 1 - distance < 0) {
        fprintf(stderr, "[LEVIATA][VM] Peek inválido!\n");
        exit(EXIT_FAILURE);
    }
    return vm->stack[vm->stack_top - 1 - distance];
}

/* ============================================================
 * VM - INICIALIZAÇÃO E LIBERAÇÃO
 * ============================================================ */
void vm_init(LvVM *vm) {
    memset(vm, 0, sizeof(LvVM));
    hmap_init(&vm->globals, LV_HASHMAP_SIZE);

    vm->gc.all_objects  = NULL;
    vm->gc.object_count = 0;
    vm->gc.gc_threshold = LV_GC_THRESHOLD;
    vm->gc.bytes_allocated = 0;

    /* Singletons imortais */
    vm->nil_singleton   = (LvObject*)calloc(1, sizeof(LvObject));
    vm->nil_singleton->type = LV_TYPE_NULL;
    vm->nil_singleton->gc_marked = 1;

    vm->true_singleton  = (LvObject*)calloc(1, sizeof(LvObject));
    vm->true_singleton->type = LV_TYPE_BOOL;
    vm->true_singleton->as.i_val = 1;
    vm->true_singleton->gc_marked = 1;

    vm->false_singleton = (LvObject*)calloc(1, sizeof(LvObject));
    vm->false_singleton->type = LV_TYPE_BOOL;
    vm->false_singleton->as.i_val = 0;
    vm->false_singleton->gc_marked = 1;

    vm->had_error = 0;
    vm->running   = 0;
}

void vm_free(LvVM *vm) {
    gc_collect(vm);

    /* Libera singletons */
    free(vm->nil_singleton);
    free(vm->true_singleton);
    free(vm->false_singleton);

    /* Libera chunks de funções */
    for (int i = 0; i < vm->func_count; i++) {
        chunk_free(vm->functions[i]);
        free(vm->functions[i]);
    }

    hmap_free(&vm->globals);
}

void vm_register_native(LvVM *vm, const char *name, LvNativeFn fn, int arity) {
    LvObject *obj = gc_alloc_object(vm, LV_TYPE_NATIVE);
    obj->gc_marked = 1; /* nativas são raízes permanentes */
    strncpy(obj->as.native.name, name, LV_MAX_IDENTIFIER - 1);
    obj->as.native.fn    = fn;
    obj->as.native.arity = arity;
    hmap_set(&vm->globals, name, obj);
}

/* ============================================================
 * HELPERS DE RUNTIME
 * ============================================================ */
static void vm_runtime_error(LvVM *vm, const char *fmt, ...) {
    va_list args;
    fprintf(stderr, "\n========================================\n");
    fprintf(stderr, " [LEVIATA] ERRO EM TEMPO DE EXECUÇÃO\n");
    fprintf(stderr, "========================================\n");

    if (vm->frame_count > 0) {
        CallFrame *fr = &vm->frames[vm->frame_count - 1];
        int line = fr->ip > 0 ? fr->chunk->lines[fr->ip - 1] : 0;
        fprintf(stderr, " -> Funcao : %s\n", fr->func_name);
        fprintf(stderr, " -> Linha  : %d\n", line);
    }

    fprintf(stderr, " -> Erro   : ");
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n========================================\n");

    vm->had_error = 1;
}

static double obj_to_float(LvObject *o) {
    if (!o) return 0.0;
    if (o->type == LV_TYPE_INT)   return (double)o->as.i_val;
    if (o->type == LV_TYPE_FLOAT) return o->as.f_val;
    return 0.0;
}

static int obj_is_truthy(LvObject *o) {
    if (!o || o->type == LV_TYPE_NULL) return 0;
    if (o->type == LV_TYPE_BOOL)       return (int)o->as.i_val;
    if (o->type == LV_TYPE_INT)        return o->as.i_val != 0;
    if (o->type == LV_TYPE_FLOAT)      return o->as.f_val != 0.0;
    if (o->type == LV_TYPE_STRING)     return o->as.string.length > 0;
    return 1;
}

static int obj_equal(LvObject *a, LvObject *b) {
    if (!a || !b) return a == b;
    if (a->type != b->type) {
        /* INT vs FLOAT */
        if ((a->type == LV_TYPE_INT || a->type == LV_TYPE_FLOAT) &&
            (b->type == LV_TYPE_INT || b->type == LV_TYPE_FLOAT))
            return obj_to_float(a) == obj_to_float(b);
        return 0;
    }
    switch (a->type) {
        case LV_TYPE_NULL:  return 1;
        case LV_TYPE_BOOL:
        case LV_TYPE_INT:   return a->as.i_val == b->as.i_val;
        case LV_TYPE_FLOAT: return a->as.f_val == b->as.f_val;
        case LV_TYPE_STRING:
            return a->as.string.length == b->as.string.length &&
                   strcmp(a->as.string.data, b->as.string.data) == 0;
        default: return a == b;
    }
}

/* ============================================================
 * VM - LOOP DE EXECUÇÃO PRINCIPAL
 * ============================================================ */
int vm_run_chunk(LvVM *vm, Chunk *main_chunk) {
    /* Prepara o frame inicial */
    if (vm->frame_count >= LV_CALL_DEPTH_MAX) {
        fprintf(stderr, "[LEVIATA] Profundidade de chamada excedida\n");
        return 0;
    }

    CallFrame *frame = &vm->frames[vm->frame_count++];
    frame->chunk     = main_chunk;
    frame->ip        = 0;
    frame->base      = vm->stack_top;
    strncpy(frame->func_name, "<principal>", LV_MAX_IDENTIFIER - 1);

    vm->running = 1;

#define READ_INST() (frame->chunk->code[frame->ip++])
#define PEEK_STACK(d) vm_peek(vm, d)
#define PUSH(v) vm_push(vm, v)
#define POP() vm_pop(vm)

    while (vm->running && !vm->had_error) {
        if (frame->ip >= frame->chunk->count) break;

        Instruction inst = READ_INST();
        uint8_t  op  = inst.opcode;
        int32_t  arg = inst.operand;

        switch (op) {
            case OP_NOP: break;

            case OP_HALT:
                vm->running = 0;
                break;

            /* ---- Carregar Constantes ---- */
            case OP_LOAD_NULL:
                PUSH(vm->nil_singleton);
                break;

            case OP_LOAD_TRUE:
                PUSH(vm->true_singleton);
                break;

            case OP_LOAD_FALSE:
                PUSH(vm->false_singleton);
                break;

            case OP_LOAD_INT: {
                ConstValue cv = frame->chunk->constants[arg];
                PUSH(gc_alloc_int(vm, cv.as.i_val));
                break;
            }

            case OP_LOAD_FLOAT: {
                ConstValue cv = frame->chunk->constants[arg];
                PUSH(gc_alloc_float(vm, cv.as.f_val));
                break;
            }

            case OP_LOAD_STRING: {
                ConstValue cv = frame->chunk->constants[arg];
                PUSH(gc_alloc_string(vm, cv.as.s_val, strlen(cv.as.s_val)));
                break;
            }

            /* ---- Variáveis Globais (HashMap O(1)) ---- */
            case OP_LOAD_GLOBAL: {
                ConstValue cv = frame->chunk->constants[arg];
                LvObject *val = hmap_get(&vm->globals, cv.as.s_val);
                if (!val) {
                    vm_runtime_error(vm, "Variável global '%s' não declarada", cv.as.s_val);
                    goto done;
                }
                PUSH(val);
                break;
            }

            case OP_STORE_GLOBAL: {
                ConstValue cv = frame->chunk->constants[arg];
                LvObject *val = PEEK_STACK(0);
                hmap_set(&vm->globals, cv.as.s_val, val);
                break;
            }

            /* ---- Variáveis Locais (acesso por índice no stack) ---- */
            case OP_LOAD_LOCAL:
                PUSH(vm->stack[frame->base + arg]);
                break;

            case OP_STORE_LOCAL:
                vm->stack[frame->base + arg] = PEEK_STACK(0);
                break;

            /* ---- Aritmética ---- */
            case OP_ADD: case OP_SUB: case OP_MUL:
            case OP_DIV: case OP_MOD: case OP_POW: {
                LvObject *b = POP();
                LvObject *a = POP();

                /* String concat com + */
                if (op == OP_ADD && a->type == LV_TYPE_STRING && b->type == LV_TYPE_STRING) {
                    size_t la = a->as.string.length, lb = b->as.string.length;
                    char *buf = (char*)malloc(la + lb + 1);
                    memcpy(buf, a->as.string.data, la);
                    memcpy(buf + la, b->as.string.data, lb);
                    buf[la + lb] = '\0';
                    LvObject *s = gc_alloc_string(vm, buf, la + lb);
                    free(buf);
                    PUSH(s);
                    break;
                }

                int both_int = (a->type == LV_TYPE_INT && b->type == LV_TYPE_INT);
                double fa = obj_to_float(a), fb = obj_to_float(b);
                int64_t ia = a->as.i_val, ib = b->as.i_val;

                if (op == OP_DIV && fb == 0.0) {
                    vm_runtime_error(vm, "Divisão por zero");
                    goto done;
                }
                if (op == OP_MOD && ib == 0) {
                    vm_runtime_error(vm, "Módulo por zero");
                    goto done;
                }

                if (both_int && op != OP_DIV && op != OP_POW) {
                    int64_t res = 0;
                    if      (op == OP_ADD) res = ia + ib;
                    else if (op == OP_SUB) res = ia - ib;
                    else if (op == OP_MUL) res = ia * ib;
                    else if (op == OP_MOD) res = ia % ib;
                    PUSH(gc_alloc_int(vm, res));
                } else {
                    double res = 0;
                    if      (op == OP_ADD) res = fa + fb;
                    else if (op == OP_SUB) res = fa - fb;
                    else if (op == OP_MUL) res = fa * fb;
                    else if (op == OP_DIV) res = fa / fb;
                    else if (op == OP_MOD) res = fmod(fa, fb);
                    else if (op == OP_POW) res = pow(fa, fb);
                    /* Se resultado é inteiro exato */
                    if (both_int || (res == (int64_t)res && op != OP_DIV))
                        PUSH(gc_alloc_int(vm, (int64_t)res));
                    else
                        PUSH(gc_alloc_float(vm, res));
                }
                break;
            }

            case OP_NEG: {
                LvObject *a = POP();
                if (a->type == LV_TYPE_INT)
                    PUSH(gc_alloc_int(vm, -a->as.i_val));
                else if (a->type == LV_TYPE_FLOAT)
                    PUSH(gc_alloc_float(vm, -a->as.f_val));
                else {
                    vm_runtime_error(vm, "Negação em tipo inválido");
                    goto done;
                }
                break;
            }

            /* ---- Comparação ---- */
            case OP_EQ: {
                LvObject *b = POP(), *a = POP();
                PUSH(gc_alloc_bool(vm, obj_equal(a, b)));
                break;
            }
            case OP_NEQ: {
                LvObject *b = POP(), *a = POP();
                PUSH(gc_alloc_bool(vm, !obj_equal(a, b)));
                break;
            }
            case OP_LT: {
                LvObject *b = POP(), *a = POP();
                PUSH(gc_alloc_bool(vm, obj_to_float(a) < obj_to_float(b)));
                break;
            }
            case OP_GT: {
                LvObject *b = POP(), *a = POP();
                PUSH(gc_alloc_bool(vm, obj_to_float(a) > obj_to_float(b)));
                break;
            }
            case OP_LTE: {
                LvObject *b = POP(), *a = POP();
                PUSH(gc_alloc_bool(vm, obj_to_float(a) <= obj_to_float(b)));
                break;
            }
            case OP_GTE: {
                LvObject *b = POP(), *a = POP();
                PUSH(gc_alloc_bool(vm, obj_to_float(a) >= obj_to_float(b)));
                break;
            }

            /* ---- Lógica ---- */
            case OP_AND: {
                LvObject *b = POP(), *a = POP();
                PUSH(gc_alloc_bool(vm, obj_is_truthy(a) && obj_is_truthy(b)));
                break;
            }
            case OP_OR: {
                LvObject *b = POP(), *a = POP();
                PUSH(gc_alloc_bool(vm, obj_is_truthy(a) || obj_is_truthy(b)));
                break;
            }
            case OP_NOT: {
                LvObject *a = POP();
                PUSH(gc_alloc_bool(vm, !obj_is_truthy(a)));
                break;
            }

            /* ---- Controle de Fluxo ---- */
            case OP_JUMP:
                frame->ip = arg;
                break;

            case OP_JUMP_IF_FALSE: {
                LvObject *cond = POP();
                if (!obj_is_truthy(cond)) frame->ip = arg;
                break;
            }

            case OP_JUMP_IF_TRUE: {
                LvObject *cond = POP();
                if (obj_is_truthy(cond)) frame->ip = arg;
                break;
            }

            /* ---- Listas ---- */
            case OP_NEW_LIST: {
                LvObject *lst = gc_alloc_list(vm);
                /* Recolhe 'arg' itens do stack */
                for (int i = arg - 1; i >= 0; i--) {
                    LvObject *item = vm->stack[vm->stack_top - arg + i];
                    if (lst->as.list.count >= lst->as.list.capacity) {
                        lst->as.list.capacity *= 2;
                        lst->as.list.items = (LvObject**)realloc(
                            lst->as.list.items,
                            lst->as.list.capacity * sizeof(LvObject*));
                    }
                    lst->as.list.items[lst->as.list.count++] = item;
                }
                vm->stack_top -= arg;
                PUSH(lst);
                break;
            }

            case OP_INDEX_GET: {
                LvObject *idx = POP();
                LvObject *obj = POP();
                if (obj->type != LV_TYPE_LIST) {
                    vm_runtime_error(vm, "Tentativa de indexar não-lista");
                    goto done;
                }
                if (idx->type != LV_TYPE_INT) {
                    vm_runtime_error(vm, "Índice deve ser inteiro");
                    goto done;
                }
                int64_t i = idx->as.i_val;
                if (i < 0 || i >= obj->as.list.count) {
                    vm_runtime_error(vm, "Índice %lld fora dos limites (tamanho: %d)",
                                     (long long)i, obj->as.list.count);
                    goto done;
                }
                PUSH(obj->as.list.items[i]);
                break;
            }

            case OP_INDEX_SET: {
                LvObject *val = POP();
                LvObject *idx = POP();
                LvObject *obj = POP();
                if (obj->type != LV_TYPE_LIST || idx->type != LV_TYPE_INT) {
                    vm_runtime_error(vm, "Atribuição em índice inválido");
                    goto done;
                }
                int64_t i = idx->as.i_val;
                if (i < 0 || i >= obj->as.list.count) {
                    vm_runtime_error(vm, "Índice %lld fora dos limites", (long long)i);
                    goto done;
                }
                obj->as.list.items[i] = val;
                PUSH(val);
                break;
            }

            case OP_LIST_PUSH: {
                LvObject *val = POP();
                LvObject *lst = PEEK_STACK(0);
                if (lst->type != LV_TYPE_LIST) {
                    vm_runtime_error(vm, "LIST_PUSH em não-lista");
                    goto done;
                }
                if (lst->as.list.count >= lst->as.list.capacity) {
                    lst->as.list.capacity *= 2;
                    lst->as.list.items = (LvObject**)realloc(
                        lst->as.list.items,
                        lst->as.list.capacity * sizeof(LvObject*));
                }
                lst->as.list.items[lst->as.list.count++] = val;
                break;
            }

            /* ---- Funções ---- */

            /*
             * OP_LOAD_FUNC — cria (ou recria) um objeto LvObject::FUNCTION
             * que encapsula o Chunk compilado registrado em vm->functions[arg].
             * Cada execução deste opcode cria um novo invólucro leve; o Chunk
             * subjacente é compartilhado e de propriedade de vm->functions[].
             */
            case OP_LOAD_FUNC: {
                int fidx = arg;
                if (fidx < 0 || fidx >= vm->func_count) {
                    vm_runtime_error(vm, "OP_LOAD_FUNC: índice de função inválido (%d)", fidx);
                    goto done;
                }
                Chunk *fn_chunk = vm->functions[fidx];
                LvObject *fn = gc_alloc_function(vm, fn_chunk,
                                                 vm->func_names[fidx],
                                                 fn_chunk->arity);
                /* Marca como raiz permanente enquanto global;
                 * o OP_STORE_GLOBAL garante a referência no HashMap. */
                PUSH(fn);
                break;
            }

            case OP_CALL: {
                int argc = arg;
                LvObject *callee = vm->stack[vm->stack_top - argc - 1];

                if (callee->type == LV_TYPE_NATIVE) {
                    /* Chamada de função nativa (FFI) */
                    LvObject **args = &vm->stack[vm->stack_top - argc];
                    LvObject *result = callee->as.native.fn(vm, argc, args);
                    vm->stack_top -= (argc + 1); /* remove args + função */
                    PUSH(result ? result : vm->nil_singleton);

                } else if (callee->type == LV_TYPE_FUNCTION) {
                    /* Verifica aridade */
                    if (argc != callee->as.function.arity) {
                        vm_runtime_error(vm,
                            "Função '%s' espera %d argumento(s), recebeu %d",
                            callee->as.function.name,
                            callee->as.function.arity, argc);
                        goto done;
                    }
                    if (vm->frame_count >= LV_CALL_DEPTH_MAX) {
                        vm_runtime_error(vm, "Stack overflow: recursão muito profunda");
                        goto done;
                    }
                    /*
                     * Convenção de chamada:
                     *   stack antes do CALL: [..., fn_obj, arg0, arg1, ...]
                     *   frame->base aponta para arg0 (= stack_top - argc)
                     *   fn_obj está em stack[base - 1]
                     *
                     * Dentro da função:
                     *   OP_LOAD_LOCAL 0  →  stack[base + 0]  = arg0
                     *   OP_LOAD_LOCAL 1  →  stack[base + 1]  = arg1
                     *
                     * OP_RETURN faz:  stack_top = base - 1   (remove fn_obj + args)
                     *                 PUSH(ret)               (resultado no lugar de fn_obj)
                     */
                    CallFrame *new_frame = &vm->frames[vm->frame_count++];
                    new_frame->chunk = callee->as.function.chunk;
                    new_frame->ip    = 0;
                    new_frame->base  = vm->stack_top - argc;
                    strncpy(new_frame->func_name, callee->as.function.name,
                            LV_MAX_IDENTIFIER - 1);
                    new_frame->func_name[LV_MAX_IDENTIFIER - 1] = '\0';
                    frame = new_frame;
                } else {
                    vm_runtime_error(vm, "Tentativa de chamar não-função");
                    goto done;
                }
                break;
            }

            case OP_RETURN: {
                LvObject *ret = POP();
                /* Remove locals do stack */
                vm->stack_top = frame->base - 1; /* -1 remove o objeto função */
                vm->frame_count--;
                PUSH(ret);
                if (vm->frame_count == 0) {
                    vm->running = 0;
                } else {
                    frame = &vm->frames[vm->frame_count - 1];
                }
                break;
            }

            /* ---- I/O ---- */
            case OP_PRINT:
            case OP_PRINTLN: {
                LvObject *val = POP();
                if (val) {
                    switch (val->type) {
                        case LV_TYPE_NULL:   printf("nulo"); break;
                        case LV_TYPE_BOOL:   printf("%s", val->as.i_val ? "verdadeiro" : "falso"); break;
                        case LV_TYPE_INT:    printf("%lld", (long long)val->as.i_val); break;
                        case LV_TYPE_FLOAT:  printf("%g", val->as.f_val); break;
                        case LV_TYPE_STRING: printf("%s", val->as.string.data); break;
                        case LV_TYPE_LIST:   printf("[lista:%d]", val->as.list.count); break;
                        case LV_TYPE_NATIVE: printf("<nativa:%s>", val->as.native.name); break;
                        default:             printf("<objeto>"); break;
                    }
                }
                if (op == OP_PRINTLN) printf("\n");
                break;
            }

            case OP_POP:
                POP();
                break;

            case OP_DUP:
                PUSH(PEEK_STACK(0));
                break;

            default:
                vm_runtime_error(vm, "Opcode desconhecido: %d", op);
                goto done;
        }
    }

done:
#undef READ_INST
#undef PUSH
#undef POP
#undef PEEK_STACK

    vm->frame_count = 0;
    vm->stack_top   = 0;
    vm->running     = 0;
    return !vm->had_error;
}
