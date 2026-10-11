/* ============================================================================
 * LEVIATA 3.8 - MAPAS / DICIONARIOS
 *
 * Mapas de usuario reutilizam LV_TYPE_MAP, mas nunca compartilham as chaves
 * internas (@...) usadas por classes e instancias. A camada tambem centraliza
 * indexacao para listas e mapas sem alterar o bytecode legado da VM.
 * ============================================================================ */
#include "leviata_maps.h"
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAPA_CAPACIDADE_INICIAL 16

static LvNativeFn anterior_obj_campo = NULL;
static LvNativeFn anterior_obj_definir = NULL;
static LvNativeFn anterior_tamanho = NULL;
static LvNativeFn anterior_contem = NULL;
static LvNativeFn anterior_remover = NULL;
static LvNativeFn anterior_limpar = NULL;
static LvNativeFn anterior_bool = NULL;

static uint32_t mapa_hash(const char *key) {
    uint32_t h = 2166136261u;
    while (key && *key) {
        h ^= (uint8_t)*key++;
        h *= 16777619u;
    }
    return h;
}

static LvObject *mapa_erro(LvVM *vm, const char *fmt, ...) {
    va_list args;
    fprintf(stderr, "[LEVIATA][Mapa] ");
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
    if (vm) vm->had_error = 1;
    return vm ? vm->nil_singleton : NULL;
}

static LvObject *mapa_get_raw(LvObject *mapa, const char *key) {
    if (!mapa || mapa->type != LV_TYPE_MAP || !key ||
        mapa->as.map.capacity <= 0 || !mapa->as.map.buckets)
        return NULL;

    uint32_t idx = mapa_hash(key) % (uint32_t)mapa->as.map.capacity;
    for (int i = 0; i < mapa->as.map.capacity; i++) {
        uint32_t slot = (idx + (uint32_t)i) % (uint32_t)mapa->as.map.capacity;
        LvMapEntry *e = &mapa->as.map.buckets[slot];
        if (!e->occupied) return NULL;
        if (strcmp(e->key, key) == 0) return e->value;
    }
    return NULL;
}

static int texto_igual(LvObject *o, const char *texto) {
    return o && o->type == LV_TYPE_STRING && texto &&
           strcmp(o->as.string.data, texto) == 0;
}

static int mapa_e_interno(LvObject *mapa) {
    if (!mapa || mapa->type != LV_TYPE_MAP) return 0;
    LvObject *tipo = mapa_get_raw(mapa, "@tipo");
    return texto_igual(tipo, "classe") || texto_igual(tipo, "instancia");
}

static int mapa_e_usuario(LvObject *mapa) {
    return mapa && mapa->type == LV_TYPE_MAP && !mapa_e_interno(mapa);
}

static int chave_valida(const char *key) {
    if (!key || !*key) return 0;
    if (key[0] == '@') return 0;
    return strlen(key) < LV_MAX_IDENTIFIER;
}

static LvObject *mapa_novo(LvVM *vm) {
    LvObject *o = gc_alloc_object(vm, LV_TYPE_MAP);
    o->as.map.capacity = MAPA_CAPACIDADE_INICIAL;
    o->as.map.count = 0;
    o->as.map.buckets = (LvMapEntry*)calloc(
        (size_t)o->as.map.capacity, sizeof(LvMapEntry));
    if (!o->as.map.buckets) {
        fprintf(stderr, "[LEVIATA][Mapa] Sem memoria para mapa\n");
        exit(EXIT_FAILURE);
    }
    return o;
}

static int mapa_set_sem_validar(LvObject *mapa, const char *key, LvObject *value);

static int mapa_rehash(LvObject *mapa, int nova_capacidade) {
    if (!mapa_e_usuario(mapa) || nova_capacidade < MAPA_CAPACIDADE_INICIAL)
        return 0;

    LvMapEntry *antigos = mapa->as.map.buckets;
    int capacidade_antiga = mapa->as.map.capacity;

    LvMapEntry *novos = (LvMapEntry*)calloc(
        (size_t)nova_capacidade, sizeof(LvMapEntry));
    if (!novos) return 0;

    mapa->as.map.buckets = novos;
    mapa->as.map.capacity = nova_capacidade;
    mapa->as.map.count = 0;

    for (int i = 0; i < capacidade_antiga; i++) {
        LvMapEntry *e = &antigos[i];
        if (!e->occupied) continue;
        if (!mapa_set_sem_validar(mapa, e->key, e->value)) {
            free(novos);
            mapa->as.map.buckets = antigos;
            mapa->as.map.capacity = capacidade_antiga;
            return 0;
        }
    }

    free(antigos);
    return 1;
}

static int mapa_set_sem_validar(LvObject *mapa, const char *key, LvObject *value) {
    if (!mapa || mapa->type != LV_TYPE_MAP || !key ||
        mapa->as.map.capacity <= 0 || !mapa->as.map.buckets)
        return 0;

    uint32_t idx = mapa_hash(key) % (uint32_t)mapa->as.map.capacity;
    for (int i = 0; i < mapa->as.map.capacity; i++) {
        uint32_t slot = (idx + (uint32_t)i) % (uint32_t)mapa->as.map.capacity;
        LvMapEntry *e = &mapa->as.map.buckets[slot];
        if (!e->occupied || strcmp(e->key, key) == 0) {
            if (!e->occupied) mapa->as.map.count++;
            e->occupied = 1;
            strncpy(e->key, key, LV_MAX_IDENTIFIER - 1);
            e->key[LV_MAX_IDENTIFIER - 1] = '\0';
            e->value = value;
            return 1;
        }
    }
    return 0;
}

static int mapa_set(LvObject *mapa, const char *key, LvObject *value) {
    if (!mapa_e_usuario(mapa) || !chave_valida(key)) return 0;

    if ((mapa->as.map.count + 1) * 10 >= mapa->as.map.capacity * 7) {
        if (!mapa_rehash(mapa, mapa->as.map.capacity * 2)) return 0;
    }
    return mapa_set_sem_validar(mapa, key, value);
}

static LvObject *mapa_get(LvObject *mapa, const char *key) {
    if (!mapa_e_usuario(mapa) || !chave_valida(key)) return NULL;
    return mapa_get_raw(mapa, key);
}

static LvObject *mapa_remover_chave(LvObject *mapa, const char *key) {
    if (!mapa_e_usuario(mapa) || !chave_valida(key)) return NULL;

    uint32_t idx = mapa_hash(key) % (uint32_t)mapa->as.map.capacity;
    for (int i = 0; i < mapa->as.map.capacity; i++) {
        uint32_t slot = (idx + (uint32_t)i) % (uint32_t)mapa->as.map.capacity;
        LvMapEntry *e = &mapa->as.map.buckets[slot];
        if (!e->occupied) return NULL;
        if (strcmp(e->key, key) != 0) continue;

        LvObject *removido = e->value;
        memset(e, 0, sizeof(*e));
        mapa->as.map.count--;

        /* Corrige a cadeia de probing aberta apos a remocao. */
        uint32_t cursor = (slot + 1u) % (uint32_t)mapa->as.map.capacity;
        while (mapa->as.map.buckets[cursor].occupied) {
            LvMapEntry movida = mapa->as.map.buckets[cursor];
            memset(&mapa->as.map.buckets[cursor], 0, sizeof(LvMapEntry));
            mapa->as.map.count--;
            mapa_set_sem_validar(mapa, movida.key, movida.value);
            cursor = (cursor + 1u) % (uint32_t)mapa->as.map.capacity;
        }
        return removido;
    }
    return NULL;
}

static int objeto_para_indice(LvObject *o, int64_t *out) {
    if (!o || !out) return 0;
    if (o->type == LV_TYPE_INT) {
        *out = o->as.i_val;
        return 1;
    }
    if (o->type == LV_TYPE_FLOAT && isfinite(o->as.f_val) &&
        trunc(o->as.f_val) == o->as.f_val) {
        *out = (int64_t)o->as.f_val;
        return 1;
    }
    return 0;
}

static LvObject *indice_obter(LvVM *vm, int argc, LvObject **args) {
    if (argc != 2 || !args[0] || !args[1])
        return mapa_erro(vm, "Indexacao exige colecao e indice");

    if (args[0]->type == LV_TYPE_LIST) {
        int64_t idx = 0;
        if (!objeto_para_indice(args[1], &idx))
            return mapa_erro(vm, "Indice de lista deve ser inteiro");
        if (idx < 0 || idx >= args[0]->as.list.count)
            return mapa_erro(vm, "Indice %lld fora dos limites da lista",
                             (long long)idx);
        return args[0]->as.list.items[idx];
    }

    if (mapa_e_usuario(args[0])) {
        if (args[1]->type != LV_TYPE_STRING)
            return mapa_erro(vm, "Chave de mapa deve ser texto");
        LvObject *valor = mapa_get(args[0], args[1]->as.string.data);
        return valor ? valor : vm->nil_singleton;
    }

    return mapa_erro(vm, "Tipo nao aceita indexacao");
}

static LvObject *indice_definir(LvVM *vm, int argc, LvObject **args) {
    if (argc != 3 || !args[0] || !args[1])
        return mapa_erro(vm, "Atribuicao indexada exige colecao, indice e valor");

    if (args[0]->type == LV_TYPE_LIST) {
        int64_t idx = 0;
        if (!objeto_para_indice(args[1], &idx))
            return mapa_erro(vm, "Indice de lista deve ser inteiro");
        if (idx < 0 || idx >= args[0]->as.list.count)
            return mapa_erro(vm, "Indice %lld fora dos limites da lista",
                             (long long)idx);
        args[0]->as.list.items[idx] = args[2];
        return args[2];
    }

    if (mapa_e_usuario(args[0])) {
        if (args[1]->type != LV_TYPE_STRING)
            return mapa_erro(vm, "Chave de mapa deve ser texto");
        if (!mapa_set(args[0], args[1]->as.string.data, args[2]))
            return mapa_erro(vm,
                "Chave invalida; use texto curto e nao iniciado por '@'");
        return args[2];
    }

    return mapa_erro(vm, "Tipo nao aceita atribuicao por indice");
}

static double como_numero(LvObject *o) {
    if (o->type == LV_TYPE_INT) return (double)o->as.i_val;
    if (o->type == LV_TYPE_FLOAT) return o->as.f_val;
    return 0.0;
}

static LvObject *indice_composto(LvVM *vm, int argc, LvObject **args) {
    if (argc != 4 || !args[2] || args[2]->type != LV_TYPE_STRING)
        return mapa_erro(vm, "Atribuicao composta indexada invalida");

    LvObject *get_args[2] = { args[0], args[1] };
    LvObject *atual = indice_obter(vm, 2, get_args);
    if (vm->had_error) return vm->nil_singleton;

    const char *op = args[2]->as.string.data;
    LvObject *rhs = args[3];
    LvObject *resultado = NULL;

    if (strcmp(op, "+") == 0 &&
        atual->type == LV_TYPE_STRING && rhs->type == LV_TYPE_STRING) {
        size_t a = atual->as.string.length;
        size_t b = rhs->as.string.length;
        char *buf = (char*)malloc(a + b + 1);
        if (!buf) return mapa_erro(vm, "Sem memoria em concatenacao");
        memcpy(buf, atual->as.string.data, a);
        memcpy(buf + a, rhs->as.string.data, b);
        buf[a + b] = '\0';
        resultado = gc_alloc_string(vm, buf, a + b);
        free(buf);
    } else if ((atual->type == LV_TYPE_INT || atual->type == LV_TYPE_FLOAT) &&
               (rhs->type == LV_TYPE_INT || rhs->type == LV_TYPE_FLOAT)) {
        int ambos_int = atual->type == LV_TYPE_INT && rhs->type == LV_TYPE_INT;
        double a = como_numero(atual);
        double b = como_numero(rhs);

        if (strcmp(op, "/") == 0 && b == 0.0)
            return mapa_erro(vm, "Divisao por zero");

        if (ambos_int && strcmp(op, "/") != 0) {
            int64_t ia = atual->as.i_val;
            int64_t ib = rhs->as.i_val;
            if (strcmp(op, "+") == 0) resultado = gc_alloc_int(vm, ia + ib);
            else if (strcmp(op, "-") == 0) resultado = gc_alloc_int(vm, ia - ib);
            else if (strcmp(op, "*") == 0) resultado = gc_alloc_int(vm, ia * ib);
        } else {
            double r = 0.0;
            if (strcmp(op, "+") == 0) r = a + b;
            else if (strcmp(op, "-") == 0) r = a - b;
            else if (strcmp(op, "*") == 0) r = a * b;
            else if (strcmp(op, "/") == 0) r = a / b;
            else return mapa_erro(vm, "Operador composto desconhecido: %s", op);
            resultado = gc_alloc_float(vm, r);
        }
    }

    if (!resultado)
        return mapa_erro(vm, "Operacao '%s=' invalida para estes valores", op);

    LvObject *set_args[3] = { args[0], args[1], resultado };
    return indice_definir(vm, 3, set_args);
}

static LvObject *mapa_construtor(LvVM *vm, int argc, LvObject **args) {
    if (argc % 2 != 0)
        return mapa_erro(vm, "mapa() recebe pares chave, valor");

    LvObject *mapa = mapa_novo(vm);
    vm_push(vm, mapa);

    for (int i = 0; i < argc; i += 2) {
        if (!args[i] || args[i]->type != LV_TYPE_STRING) {
            vm_pop(vm);
            return mapa_erro(vm, "Chaves de mapa devem ser texto");
        }
        if (!mapa_set(mapa, args[i]->as.string.data, args[i + 1])) {
            vm_pop(vm);
            return mapa_erro(vm,
                "Chave '%s' invalida; limite de %d caracteres e '@' reservado",
                args[i]->as.string.data, LV_MAX_IDENTIFIER - 1);
        }
    }

    vm_pop(vm);
    return mapa;
}

static LvObject *mapa_chaves(LvVM *vm, int argc, LvObject **args) {
    if (argc != 1 || !mapa_e_usuario(args[0]))
        return mapa_erro(vm, "chaves() espera um mapa");

    LvObject *lista = gc_alloc_list(vm);
    vm_push(vm, lista);
    for (int i = 0; i < args[0]->as.map.capacity; i++) {
        LvMapEntry *e = &args[0]->as.map.buckets[i];
        if (!e->occupied) continue;
        if (lista->as.list.count >= lista->as.list.capacity) {
            lista->as.list.capacity *= 2;
            lista->as.list.items = (LvObject**)realloc(
                lista->as.list.items,
                (size_t)lista->as.list.capacity * sizeof(LvObject*));
        }
        lista->as.list.items[lista->as.list.count++] =
            gc_alloc_string(vm, e->key, strlen(e->key));
    }
    vm_pop(vm);
    return lista;
}

static LvObject *mapa_valores(LvVM *vm, int argc, LvObject **args) {
    if (argc != 1 || !mapa_e_usuario(args[0]))
        return mapa_erro(vm, "valores() espera um mapa");

    LvObject *lista = gc_alloc_list(vm);
    for (int i = 0; i < args[0]->as.map.capacity; i++) {
        LvMapEntry *e = &args[0]->as.map.buckets[i];
        if (!e->occupied) continue;
        if (lista->as.list.count >= lista->as.list.capacity) {
            lista->as.list.capacity *= 2;
            lista->as.list.items = (LvObject**)realloc(
                lista->as.list.items,
                (size_t)lista->as.list.capacity * sizeof(LvObject*));
        }
        lista->as.list.items[lista->as.list.count++] = e->value;
    }
    return lista;
}

static LvObject *mapa_tamanho(LvVM *vm, int argc, LvObject **args) {
    if (argc == 1 && mapa_e_usuario(args[0]))
        return gc_alloc_int(vm, args[0]->as.map.count);
    return anterior_tamanho ? anterior_tamanho(vm, argc, args) : vm->nil_singleton;
}

static LvObject *mapa_contem(LvVM *vm, int argc, LvObject **args) {
    if (argc == 2 && mapa_e_usuario(args[0])) {
        if (!args[1] || args[1]->type != LV_TYPE_STRING)
            return gc_alloc_bool(vm, 0);
        return gc_alloc_bool(vm,
            mapa_get(args[0], args[1]->as.string.data) != NULL);
    }
    return anterior_contem ? anterior_contem(vm, argc, args) : gc_alloc_bool(vm, 0);
}

static LvObject *mapa_remover(LvVM *vm, int argc, LvObject **args) {
    if (argc == 2 && mapa_e_usuario(args[0])) {
        if (!args[1] || args[1]->type != LV_TYPE_STRING)
            return vm->nil_singleton;
        LvObject *removido = mapa_remover_chave(args[0], args[1]->as.string.data);
        return removido ? removido : vm->nil_singleton;
    }
    return anterior_remover ? anterior_remover(vm, argc, args) : vm->nil_singleton;
}

static LvObject *mapa_limpar(LvVM *vm, int argc, LvObject **args) {
    if (argc == 1 && mapa_e_usuario(args[0])) {
        memset(args[0]->as.map.buckets, 0,
               (size_t)args[0]->as.map.capacity * sizeof(LvMapEntry));
        args[0]->as.map.count = 0;
        return args[0];
    }
    return anterior_limpar ? anterior_limpar(vm, argc, args) : vm->nil_singleton;
}

static LvObject *mapa_bool(LvVM *vm, int argc, LvObject **args) {
    if (argc == 1 && mapa_e_usuario(args[0]))
        return gc_alloc_bool(vm, args[0]->as.map.count > 0);
    return anterior_bool ? anterior_bool(vm, argc, args) : gc_alloc_bool(vm, 0);
}

static LvObject *mapa_obj_campo(LvVM *vm, int argc, LvObject **args) {
    if (argc == 2 && mapa_e_usuario(args[0]) && args[1] &&
        args[1]->type == LV_TYPE_STRING) {
        LvObject *valor = mapa_get(args[0], args[1]->as.string.data);
        return valor ? valor : vm->nil_singleton;
    }
    return anterior_obj_campo ? anterior_obj_campo(vm, argc, args) : vm->nil_singleton;
}

static LvObject *mapa_obj_definir(LvVM *vm, int argc, LvObject **args) {
    if (argc == 3 && mapa_e_usuario(args[0]) && args[1] &&
        args[1]->type == LV_TYPE_STRING) {
        if (!mapa_set(args[0], args[1]->as.string.data, args[2]))
            return mapa_erro(vm, "Campo de mapa invalido: %s", args[1]->as.string.data);
        return args[2];
    }
    return anterior_obj_definir ? anterior_obj_definir(vm, argc, args) : vm->nil_singleton;
}

static LvNativeFn capturar(LvVM *vm, const char *nome) {
    LvObject *o = hmap_get(&vm->globals, nome);
    return (o && o->type == LV_TYPE_NATIVE) ? o->as.native.fn : NULL;
}

void leviata_maps_register(LvVM *vm) {
    if (!vm) return;

    anterior_obj_campo = capturar(vm, "@obj.campo");
    anterior_obj_definir = capturar(vm, "@obj.definir");
    anterior_tamanho = capturar(vm, "tamanho");
    anterior_contem = capturar(vm, "contem");
    anterior_remover = capturar(vm, "remover");
    anterior_limpar = capturar(vm, "limpar");
    anterior_bool = capturar(vm, "bool");

    vm_register_native(vm, "mapa", mapa_construtor, -1);
    vm_register_native(vm, "chaves", mapa_chaves, 1);
    vm_register_native(vm, "valores", mapa_valores, 1);

    vm_register_native(vm, "@indice.obter", indice_obter, 2);
    vm_register_native(vm, "@indice.definir", indice_definir, 3);
    vm_register_native(vm, "@indice.composto", indice_composto, 4);

    vm_register_native(vm, "@obj.campo", mapa_obj_campo, 2);
    vm_register_native(vm, "@obj.definir", mapa_obj_definir, 3);

    vm_register_native(vm, "tamanho", mapa_tamanho, 1);
    vm_register_native(vm, "contem", mapa_contem, 2);
    vm_register_native(vm, "remover", mapa_remover, 2);
    vm_register_native(vm, "limpar", mapa_limpar, 1);
    vm_register_native(vm, "bool", mapa_bool, 1);
}
