/* ============================================================================
 * LEVIATA 3.5 - RUNTIME DE SUPER
 *
 * `super.metodo(...)` precisa procurar o método começando na classe pai da
 * classe onde o código foi declarado. O compilador 3.5 resolve a classe atual
 * e chama a nativa interna @classe.super_metodo.
 * ============================================================================ */
#include "leviata_super.h"
#include <stdint.h>
#include <string.h>

static uint32_t super_hash(const char *key) {
    uint32_t h = 2166136261u;
    while (*key) {
        h ^= (uint8_t)*key++;
        h *= 16777619u;
    }
    return h;
}

static LvObject *super_mapa_get(LvObject *mapa, const char *key) {
    if (!mapa || mapa->type != LV_TYPE_MAP || !key ||
        mapa->as.map.capacity <= 0 || !mapa->as.map.buckets)
        return NULL;

    uint32_t idx = super_hash(key) % (uint32_t)mapa->as.map.capacity;
    for (int i = 0; i < mapa->as.map.capacity; i++) {
        uint32_t slot = (idx + (uint32_t)i) % (uint32_t)mapa->as.map.capacity;
        LvMapEntry *e = &mapa->as.map.buckets[slot];
        if (!e->occupied) return NULL;
        if (strcmp(e->key, key) == 0) return e->value;
    }
    return NULL;
}

static int super_texto_igual(LvObject *o, const char *texto) {
    return o && o->type == LV_TYPE_STRING && texto &&
           strcmp(o->as.string.data, texto) == 0;
}

static int super_e_classe(LvObject *o) {
    return o && o->type == LV_TYPE_MAP &&
           super_texto_igual(super_mapa_get(o, "@tipo"), "classe");
}

static LvObject *super_busca_metodo(LvObject *classe, const char *nome) {
    char key[LV_MAX_IDENTIFIER];
    int n = snprintf(key, sizeof(key), "@metodo:%s", nome ? nome : "");
    if (n < 0 || (size_t)n >= sizeof(key)) return NULL;

    for (LvObject *atual = classe; super_e_classe(atual); ) {
        LvObject *metodo = super_mapa_get(atual, key);
        if (metodo) return metodo;
        LvObject *pai = super_mapa_get(atual, "@pai");
        atual = super_e_classe(pai) ? pai : NULL;
    }
    return NULL;
}

static LvObject *super_metodo(LvVM *vm, int argc, LvObject **args) {
    if (argc != 2 || !super_e_classe(args[0]) ||
        !args[1] || args[1]->type != LV_TYPE_STRING)
        return vm->nil_singleton;

    LvObject *pai = super_mapa_get(args[0], "@pai");
    if (!super_e_classe(pai))
        return vm->nil_singleton;

    LvObject *metodo = super_busca_metodo(pai, args[1]->as.string.data);
    return metodo ? metodo : vm->nil_singleton;
}

void leviata_super_register(LvVM *vm) {
    if (!vm) return;
    vm_register_native(vm, "@classe.super_metodo", super_metodo, 2);
}
