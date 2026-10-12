/* ============================================================================
 * LEVIATA 3.10 - PARES PARA LISTAS E MAPAS
 *
 * A sintaxe `para a, b em colecao` introduzida no 3.9 continua sendo
 * normalizada para `pares(colecao)`. Nesta versao, `pares()` deixa de ser
 * exclusivo de mapas:
 *
 *   mapa  -> [chave, valor]
 *   lista -> [indice, valor]
 *
 * A lista de pares e um snapshot, portanto mutacoes na colecao original
 * durante o laco nao alteram o conjunto percorrido.
 * ============================================================================ */
#define leviata_maps_register leviata_maps_register_v39
#include "leviata_maps_v39.c"
#undef leviata_maps_register

static LvObject *v310_pares(LvVM *vm, int argc, LvObject **args) {
    if (argc != 1 || !args[0])
        return mapa_erro(vm, "pares() espera uma lista ou mapa");

    if (mapa_e_usuario(args[0]))
        return mapa_pares(vm, argc, args);

    if (args[0]->type == LV_TYPE_LIST) {
        LvObject *resultado = gc_alloc_list(vm);
        vm_push(vm, resultado);

        int quantidade = args[0]->as.list.count;
        for (int i = 0; i < quantidade; i++) {
            LvObject *par = gc_alloc_list(vm);
            vm_push(vm, par);

            LvObject *indice = gc_alloc_int(vm, i);
            v39_lista_adicionar(par, indice);
            v39_lista_adicionar(par, args[0]->as.list.items[i]);
            v39_lista_adicionar(resultado, par);

            vm_pop(vm);
        }

        vm_pop(vm);
        return resultado;
    }

    return mapa_erro(vm, "pares() espera uma lista ou mapa");
}

void leviata_maps_register(LvVM *vm) {
    leviata_maps_register_v39(vm);
    vm_register_native(vm, "pares", v310_pares, 1);
}
