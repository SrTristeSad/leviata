/* ============================================================================
 * LEVIATA 3.10 - PARES PARA LISTAS E MAPAS
 *
 * A sintaxe `para a, b em colecao` introduzida no 3.9 continua sendo
 * normalizada para `pares(colecao)`. Nesta versao, `pares()` aceita:
 *
 *   mapa  -> [chave, valor]
 *   lista -> [indice, valor]
 *
 * A camada compoe diretamente sobre mapas 3.8. Assim evitamos empilhar dois
 * wrappers que redefinem o mesmo ponto de registro. O resultado e sempre um
 * snapshot da colecao no inicio da iteracao.
 * ============================================================================ */
#define leviata_maps_register leviata_maps_register_v38
#include "leviata_maps.c"
#undef leviata_maps_register

static void v310_lista_adicionar(LvObject *lista, LvObject *valor) {
    if (!lista || lista->type != LV_TYPE_LIST) return;
    if (lista->as.list.count >= lista->as.list.capacity) {
        lista->as.list.capacity *= 2;
        lista->as.list.items = (LvObject**)realloc(
            lista->as.list.items,
            (size_t)lista->as.list.capacity * sizeof(LvObject*));
    }
    lista->as.list.items[lista->as.list.count++] = valor;
}

static LvObject *v310_pares_mapa(LvVM *vm, LvObject *mapa) {
    LvObject *resultado = gc_alloc_list(vm);
    vm_push(vm, resultado);

    for (int i = 0; i < mapa->as.map.capacity; i++) {
        LvMapEntry *e = &mapa->as.map.buckets[i];
        if (!e->occupied) continue;

        LvObject *par = gc_alloc_list(vm);
        vm_push(vm, par);

        LvObject *chave = gc_alloc_string(vm, e->key, strlen(e->key));
        v310_lista_adicionar(par, chave);
        v310_lista_adicionar(par, e->value);
        v310_lista_adicionar(resultado, par);

        vm_pop(vm);
    }

    vm_pop(vm);
    return resultado;
}

static LvObject *v310_pares_lista(LvVM *vm, LvObject *lista) {
    LvObject *resultado = gc_alloc_list(vm);
    vm_push(vm, resultado);

    const int quantidade = lista->as.list.count;
    for (int i = 0; i < quantidade; i++) {
        LvObject *par = gc_alloc_list(vm);
        vm_push(vm, par);

        LvObject *indice = gc_alloc_int(vm, i);
        v310_lista_adicionar(par, indice);
        v310_lista_adicionar(par, lista->as.list.items[i]);
        v310_lista_adicionar(resultado, par);

        vm_pop(vm);
    }

    vm_pop(vm);
    return resultado;
}

static LvObject *v310_pares(LvVM *vm, int argc, LvObject **args) {
    if (argc != 1 || !args[0])
        return mapa_erro(vm, "pares() espera uma lista ou mapa");

    if (mapa_e_usuario(args[0]))
        return v310_pares_mapa(vm, args[0]);

    if (args[0]->type == LV_TYPE_LIST)
        return v310_pares_lista(vm, args[0]);

    return mapa_erro(vm, "pares() espera uma lista ou mapa");
}

void leviata_maps_register(LvVM *vm) {
    leviata_maps_register_v38(vm);
    vm_register_native(vm, "pares", v310_pares, 1);
}
