/* ============================================================================
 * LEVIATA 3.9 - EXTENSAO DE MAPAS PARA ITERACAO
 * ============================================================================ */
#define leviata_maps_register leviata_maps_register_v38
#include "leviata_maps.c"
#undef leviata_maps_register

static void v39_lista_adicionar(LvObject *lista, LvObject *valor) {
    if (!lista || lista->type != LV_TYPE_LIST) return;
    if (lista->as.list.count >= lista->as.list.capacity) {
        lista->as.list.capacity *= 2;
        lista->as.list.items = (LvObject**)realloc(
            lista->as.list.items,
            (size_t)lista->as.list.capacity * sizeof(LvObject*));
    }
    lista->as.list.items[lista->as.list.count++] = valor;
}

static LvObject *mapa_pares(LvVM *vm, int argc, LvObject **args) {
    if (argc != 1 || !mapa_e_usuario(args[0]))
        return mapa_erro(vm, "pares() espera um mapa");

    LvObject *resultado = gc_alloc_list(vm);
    vm_push(vm, resultado);

    for (int i = 0; i < args[0]->as.map.capacity; i++) {
        LvMapEntry *e = &args[0]->as.map.buckets[i];
        if (!e->occupied) continue;

        LvObject *par = gc_alloc_list(vm);
        vm_push(vm, par);

        LvObject *chave = gc_alloc_string(vm, e->key, strlen(e->key));
        v39_lista_adicionar(par, chave);
        v39_lista_adicionar(par, e->value);
        v39_lista_adicionar(resultado, par);

        vm_pop(vm);
    }

    vm_pop(vm);
    return resultado;
}

void leviata_maps_register(LvVM *vm) {
    leviata_maps_register_v38(vm);
    vm_register_native(vm, "pares", mapa_pares, 1);
}
