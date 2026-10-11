/* ============================================================================
 * LEVIATA 3.2 - RUNTIME DE CLASSES E OBJETOS DE USUÁRIO
 *
 * Classes e instâncias usam LV_TYPE_MAP, que já é rastreado pelo GC.
 * As chaves reservadas começam com '@', símbolo que não pode iniciar um
 * identificador Leviata e portanto não colide com campos do usuário.
 * ============================================================================ */
#include "leviata_classes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CLASS_MAP_INITIAL_CAPACITY 32

static uint32_t mapa_hash(const char *key) {
    uint32_t h = 2166136261u;
    while (*key) {
        h ^= (uint8_t)*key++;
        h *= 16777619u;
    }
    return h;
}

static LvObject *mapa_novo(LvVM *vm) {
    LvObject *o = gc_alloc_object(vm, LV_TYPE_MAP);
    o->as.map.capacity = CLASS_MAP_INITIAL_CAPACITY;
    o->as.map.count = 0;
    o->as.map.buckets = (LvMapEntry*)calloc(
        (size_t)o->as.map.capacity, sizeof(LvMapEntry));
    if (!o->as.map.buckets) {
        fprintf(stderr, "[LEVIATA][Classe] Sem memória para mapa de objeto\n");
        exit(EXIT_FAILURE);
    }
    return o;
}

static LvObject *mapa_get(LvObject *mapa, const char *key) {
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

static void mapa_rehash(LvObject *mapa, int nova_capacidade) {
    LvMapEntry *antigos = mapa->as.map.buckets;
    int capacidade_antiga = mapa->as.map.capacity;

    mapa->as.map.buckets = (LvMapEntry*)calloc(
        (size_t)nova_capacidade, sizeof(LvMapEntry));
    if (!mapa->as.map.buckets) {
        mapa->as.map.buckets = antigos;
        return;
    }

    mapa->as.map.capacity = nova_capacidade;
    mapa->as.map.count = 0;

    for (int i = 0; i < capacidade_antiga; i++) {
        LvMapEntry *old = &antigos[i];
        if (!old->occupied) continue;

        uint32_t idx = mapa_hash(old->key) % (uint32_t)mapa->as.map.capacity;
        for (int j = 0; j < mapa->as.map.capacity; j++) {
            uint32_t slot = (idx + (uint32_t)j) % (uint32_t)mapa->as.map.capacity;
            LvMapEntry *e = &mapa->as.map.buckets[slot];
            if (e->occupied) continue;
            *e = *old;
            mapa->as.map.count++;
            break;
        }
    }

    free(antigos);
}

static int mapa_set(LvObject *mapa, const char *key, LvObject *value) {
    if (!mapa || mapa->type != LV_TYPE_MAP || !key) return 0;

    if ((mapa->as.map.count + 1) * 10 >= mapa->as.map.capacity * 7)
        mapa_rehash(mapa, mapa->as.map.capacity * 2);

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

static int texto_igual(LvObject *o, const char *texto) {
    return o && o->type == LV_TYPE_STRING && texto &&
           strcmp(o->as.string.data, texto) == 0;
}

static int e_classe(LvObject *o) {
    return o && o->type == LV_TYPE_MAP &&
           texto_igual(mapa_get(o, "@tipo"), "classe");
}

static int e_instancia(LvObject *o) {
    return o && o->type == LV_TYPE_MAP &&
           texto_igual(mapa_get(o, "@tipo"), "instancia");
}

static LvObject *classe_da_instancia(LvObject *instancia) {
    if (!e_instancia(instancia)) return NULL;
    LvObject *classe = mapa_get(instancia, "@classe");
    return e_classe(classe) ? classe : NULL;
}

static int chave_reservada(char *out, size_t out_size,
                           const char *prefixo, const char *nome) {
    int n = snprintf(out, out_size, "@%s:%s", prefixo, nome ? nome : "");
    return n >= 0 && (size_t)n < out_size;
}

static LvObject *buscar_metodo_classe(LvObject *classe, const char *nome) {
    char key[LV_MAX_IDENTIFIER];
    if (!chave_reservada(key, sizeof(key), "metodo", nome)) return NULL;

    for (LvObject *atual = classe; e_classe(atual); ) {
        LvObject *metodo = mapa_get(atual, key);
        if (metodo) return metodo;
        LvObject *pai = mapa_get(atual, "@pai");
        atual = e_classe(pai) ? pai : NULL;
    }
    return NULL;
}

static LvObject *clonar_padrao(LvVM *vm, LvObject *valor) {
    if (!valor) return vm->nil_singleton;

    if (valor->type != LV_TYPE_LIST)
        return valor;

    LvObject *copia = gc_alloc_list(vm);
    vm_push(vm, copia);

    for (int i = 0; i < valor->as.list.count; i++) {
        LvObject *item = clonar_padrao(vm, valor->as.list.items[i]);
        if (copia->as.list.count >= copia->as.list.capacity) {
            copia->as.list.capacity *= 2;
            copia->as.list.items = (LvObject**)realloc(
                copia->as.list.items,
                (size_t)copia->as.list.capacity * sizeof(LvObject*));
        }
        copia->as.list.items[copia->as.list.count++] = item;
    }

    vm_pop(vm);
    return copia;
}

static void copiar_campos_classe(LvVM *vm, LvObject *classe, LvObject *instancia) {
    if (!e_classe(classe)) return;

    LvObject *pai = mapa_get(classe, "@pai");
    if (e_classe(pai))
        copiar_campos_classe(vm, pai, instancia);

    const char *prefixo = "@campo:";
    size_t prefixo_len = strlen(prefixo);

    for (int i = 0; i < classe->as.map.capacity; i++) {
        LvMapEntry *e = &classe->as.map.buckets[i];
        if (!e->occupied) continue;
        if (strncmp(e->key, prefixo, prefixo_len) != 0) continue;

        const char *nome = e->key + prefixo_len;
        LvObject *valor = clonar_padrao(vm, e->value);
        mapa_set(instancia, nome, valor);
    }
}

/* ============================================================
 * Nativas internas geradas pelo compilador
 * ============================================================ */
static LvObject *classe_criar(LvVM *vm, int argc, LvObject **args) {
    if (argc < 1 || !args[0] || args[0]->type != LV_TYPE_STRING)
        return vm->nil_singleton;

    LvObject *classe = mapa_novo(vm);
    vm_push(vm, classe);

    mapa_set(classe, "@tipo", gc_alloc_string(vm, "classe", 6));
    mapa_set(classe, "@nome", args[0]);

    LvObject *pai = (argc >= 2 && e_classe(args[1])) ? args[1] : vm->nil_singleton;
    mapa_set(classe, "@pai", pai);

    vm_pop(vm);
    return classe;
}

static LvObject *classe_campo(LvVM *vm, int argc, LvObject **args) {
    if (argc < 3 || !e_classe(args[0]) ||
        !args[1] || args[1]->type != LV_TYPE_STRING)
        return gc_alloc_bool(vm, 0);

    char key[LV_MAX_IDENTIFIER];
    if (!chave_reservada(key, sizeof(key), "campo", args[1]->as.string.data))
        return gc_alloc_bool(vm, 0);

    return gc_alloc_bool(vm, mapa_set(args[0], key, args[2]));
}

static LvObject *classe_metodo(LvVM *vm, int argc, LvObject **args) {
    if (argc < 3 || !e_classe(args[0]) ||
        !args[1] || args[1]->type != LV_TYPE_STRING ||
        !args[2] || args[2]->type != LV_TYPE_FUNCTION)
        return gc_alloc_bool(vm, 0);

    char key[LV_MAX_IDENTIFIER];
    if (!chave_reservada(key, sizeof(key), "metodo", args[1]->as.string.data))
        return gc_alloc_bool(vm, 0);

    return gc_alloc_bool(vm, mapa_set(args[0], key, args[2]));
}

static LvObject *classe_novo(LvVM *vm, int argc, LvObject **args) {
    if (argc < 1 || !e_classe(args[0])) return vm->nil_singleton;

    LvObject *instancia = mapa_novo(vm);
    vm_push(vm, instancia);

    mapa_set(instancia, "@tipo", gc_alloc_string(vm, "instancia", 9));
    mapa_set(instancia, "@classe", args[0]);
    copiar_campos_classe(vm, args[0], instancia);

    vm_pop(vm);
    return instancia;
}

static LvObject *obj_campo(LvVM *vm, int argc, LvObject **args) {
    if (argc < 2 || !e_instancia(args[0]) ||
        !args[1] || args[1]->type != LV_TYPE_STRING)
        return vm->nil_singleton;

    LvObject *valor = mapa_get(args[0], args[1]->as.string.data);
    return valor ? valor : vm->nil_singleton;
}

static LvObject *obj_definir(LvVM *vm, int argc, LvObject **args) {
    if (argc < 3 || !e_instancia(args[0]) ||
        !args[1] || args[1]->type != LV_TYPE_STRING)
        return vm->nil_singleton;

    if (!mapa_set(args[0], args[1]->as.string.data, args[2]))
        return vm->nil_singleton;
    return args[2];
}

static LvObject *obj_metodo(LvVM *vm, int argc, LvObject **args) {
    if (argc < 2 || !e_instancia(args[0]) ||
        !args[1] || args[1]->type != LV_TYPE_STRING)
        return vm->nil_singleton;

    LvObject *classe = classe_da_instancia(args[0]);
    LvObject *metodo = buscar_metodo_classe(classe, args[1]->as.string.data);
    return metodo ? metodo : vm->nil_singleton;
}

static LvObject *classe_nome(LvVM *vm, int argc, LvObject **args) {
    if (argc < 1) return vm->nil_singleton;

    LvObject *classe = e_classe(args[0]) ? args[0] : classe_da_instancia(args[0]);
    if (!classe) return vm->nil_singleton;

    LvObject *nome = mapa_get(classe, "@nome");
    return nome ? nome : vm->nil_singleton;
}

void leviata_classes_register(LvVM *vm) {
    if (!vm) return;

    vm_register_native(vm, "@classe.criar", classe_criar, -1);
    vm_register_native(vm, "@classe.campo", classe_campo, 3);
    vm_register_native(vm, "@classe.metodo", classe_metodo, 3);
    vm_register_native(vm, "@classe.novo", classe_novo, 1);

    vm_register_native(vm, "@obj.campo", obj_campo, 2);
    vm_register_native(vm, "@obj.definir", obj_definir, 3);
    vm_register_native(vm, "@obj.metodo", obj_metodo, 2);
    vm_register_native(vm, "@obj.classe_nome", classe_nome, 1);
}