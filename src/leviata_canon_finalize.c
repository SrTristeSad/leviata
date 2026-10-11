/* ============================================================================
 * LEVIATA 3 - FINALIZAÇÃO SEGURA DA TABELA GLOBAL
 * ============================================================================ */
#include "leviata_canon.h"
#include <stdlib.h>
#include <string.h>

static int nome_legado(const char *nome) {
    static const char *legados[] = {
        "escrever", "imprimir", "ler",
        "para_texto", "para_inteiro", "para_decimal",
        "ler_arquivo", "escrever_arquivo", "adicionar_arquivo",
        "absoluto", "cosseno", "tangente", "logaritmo", "logaritmo10",
        "maximo", "minimo", "aleatorio_inteiro",
        "fatiar", "comeca_com", "termina_com",
        "lista_nova", "lista_adicionar", "lista_remover", "lista_contem",
        "janela_criar", "janela_fechar", "desenhar_retangulo", "desenhar_texto",
        "jogo_iniciar", "jogo_fechar", "jogo_deve_fechar",
        "jogo_iniciar_frame", "jogo_finalizar_frame", "jogo_limpar_fundo",
        "jogo_desenhar_sprite", "jogo_tecla", "jogo_mouse_x", "jogo_mouse_y",
        "jogo_delta", "jogo_desenhar_retangulo", "jogo_desenhar_circulo",
        "jogo_desenhar_texto", "jogo_tecla_apertada", "jogo_tecla_solta",
        "jogo_mouse_botao", "jogo_fps", "jogo_audio_iniciar",
        "jogo_tocar_som", "jogo_tocar_musica", "jogo_volume",
        "jogo_camera_configurar", "jogo_camera_seguir", "jogo_camera_suavidade",
        "jogo_camera_iniciar", "jogo_camera_finalizar", "jogo_camera_limites",
        "jogo_camera_limites_limpar", "jogo_colisao_retangulos",
        "jogo_ponto_no_retangulo", "jogo_colisao_circulos",
        "jogo_desenhar_sprite_regiao", "jogo_animacao_frame",
        "jogo_desenhar_animacao", "jogo_desenhar_tilemap", "jogo_colisao_tilemap",
        "cena_mudar", "cena_atual", "cena_anterior", "cena_e", "cena_versao",
        "entidade_criar", "entidade_destruir", "entidade_buscar",
        "entidade_posicionar", "entidade_tamanho", "entidade_velocidade",
        "entidade_fisica", "entidade_colisor", "entidade_gatilho",
        "entidade_colide", "entidade_distancia", "entidade_atualizar",
        "entidade_atualizar_tilemap", "entidade_sprite", "entidade_sprite_remover",
        "entidade_sprite_inverter_x", "entidade_animacao_reproduzir",
        "entidade_animacao_frame", "entidade_desenhar_sprite", "entidade_desenhar",
        "entidade_x", "entidade_y", "entidade_vx", "entidade_vy",
        "entidade_no_chao", "entidade_nome", "entidade_tem_componente",
        "entidade_quantidade", "mapa_carregar", "mapa_largura", "mapa_altura",
        NULL
    };

    for (int i = 0; legados[i] != NULL; i++) {
        if (strcmp(nome, legados[i]) == 0) return 1;
    }
    return 0;
}

void leviata_canon_finalize(LvVM *vm) {
    if (!vm || !vm->globals.entries || vm->globals.capacity <= 0) return;

    HashMap antiga = vm->globals;
    HashMap nova;
    hmap_init(&nova, antiga.capacity);

    for (int i = 0; i < antiga.capacity; i++) {
        HMapEntry *e = &antiga.entries[i];
        if (!e->in_use) continue;
        if (nome_legado(e->key)) continue;
        hmap_set(&nova, e->key, e->value);
    }

    free(antiga.entries);
    vm->globals = nova;
}
