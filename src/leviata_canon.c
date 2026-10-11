/* ============================================================================
 * LEVIATA 3 - SUPERFÍCIE CANÔNICA DA BIBLIOTECA
 *
 * A implementação interna continua reutilizando os bindings maduros do motor.
 * O código Leviata recebe aqui uma API curta, uniforme e sem aliases históricos.
 * ============================================================================ */
#include "leviata_canon.h"
#include "leviata_ffi.h"
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

static double numero(LvObject *o) {
    if (!o) return 0.0;
    if (o->type == LV_TYPE_FLOAT) return o->as.f_val;
    if (o->type == LV_TYPE_INT || o->type == LV_TYPE_BOOL)
        return (double)o->as.i_val;
    return 0.0;
}

static int verdade(LvObject *o) {
    if (!o || o->type == LV_TYPE_NULL) return 0;
    switch (o->type) {
        case LV_TYPE_BOOL:
        case LV_TYPE_INT:
            return o->as.i_val != 0;
        case LV_TYPE_FLOAT:
            return o->as.f_val != 0.0;
        case LV_TYPE_STRING:
            return o->as.string.length > 0;
        case LV_TYPE_LIST:
            return o->as.list.count > 0;
        default:
            return 1;
    }
}

static int iguais(LvObject *a, LvObject *b) {
    if (!a || !b) return a == b;

    if ((a->type == LV_TYPE_INT || a->type == LV_TYPE_FLOAT) &&
        (b->type == LV_TYPE_INT || b->type == LV_TYPE_FLOAT))
        return numero(a) == numero(b);

    if (a->type != b->type) return 0;

    switch (a->type) {
        case LV_TYPE_NULL: return 1;
        case LV_TYPE_BOOL:
        case LV_TYPE_INT: return a->as.i_val == b->as.i_val;
        case LV_TYPE_FLOAT: return a->as.f_val == b->as.f_val;
        case LV_TYPE_STRING:
            return a->as.string.length == b->as.string.length &&
                   strcmp(a->as.string.data, b->as.string.data) == 0;
        default:
            return a == b;
    }
}

static int indice(LvObject *o, int64_t *out) {
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

static void lista_garante(LvObject *lista, int necessario) {
    if (!lista || lista->type != LV_TYPE_LIST) return;
    if (lista->as.list.capacity <= 0) lista->as.list.capacity = 8;
    while (lista->as.list.capacity < necessario)
        lista->as.list.capacity *= 2;
    lista->as.list.items = (LvObject**)realloc(
        lista->as.list.items,
        (size_t)lista->as.list.capacity * sizeof(LvObject*));
}

/* ============================================================
 * Funções canônicas novas
 * ============================================================ */
static LvObject *canon_bool(LvVM *vm, int argc, LvObject **args) {
    if (argc < 1) return gc_alloc_bool(vm, 0);
    return gc_alloc_bool(vm, verdade(args[0]));
}

static LvObject *canon_tipo(LvVM *vm, int argc, LvObject **args) {
    const char *nome = "nulo";
    if (argc >= 1 && args[0]) {
        switch (args[0]->type) {
            case LV_TYPE_NULL: nome = "nulo"; break;
            case LV_TYPE_BOOL: nome = "bool"; break;
            case LV_TYPE_INT: nome = "int"; break;
            case LV_TYPE_FLOAT: nome = "dec"; break;
            case LV_TYPE_STRING: nome = "txt"; break;
            case LV_TYPE_LIST: nome = "lista"; break;
            case LV_TYPE_MAP: nome = "mapa"; break;
            case LV_TYPE_FUNCTION: nome = "funcao"; break;
            case LV_TYPE_NATIVE: nome = "nativa"; break;
            case LV_TYPE_OBJECT: nome = "objeto"; break;
            default: nome = "objeto"; break;
        }
    }
    return gc_alloc_string(vm, nome, strlen(nome));
}

static LvObject *canon_contem(LvVM *vm, int argc, LvObject **args) {
    if (argc < 2 || !args[0]) return gc_alloc_bool(vm, 0);

    if (args[0]->type == LV_TYPE_STRING &&
        args[1] && args[1]->type == LV_TYPE_STRING) {
        return gc_alloc_bool(vm,
            strstr(args[0]->as.string.data, args[1]->as.string.data) != NULL);
    }

    if (args[0]->type == LV_TYPE_LIST) {
        for (int i = 0; i < args[0]->as.list.count; i++) {
            if (iguais(args[0]->as.list.items[i], args[1]))
                return gc_alloc_bool(vm, 1);
        }
    }

    return gc_alloc_bool(vm, 0);
}

static LvObject *canon_inserir(LvVM *vm, int argc, LvObject **args) {
    if (argc < 3 || !args[0] || args[0]->type != LV_TYPE_LIST)
        return gc_alloc_bool(vm, 0);

    int64_t pos = 0;
    if (!indice(args[1], &pos) || pos < 0 || pos > args[0]->as.list.count)
        return gc_alloc_bool(vm, 0);

    LvObject *lista = args[0];
    lista_garante(lista, lista->as.list.count + 1);
    memmove(&lista->as.list.items[pos + 1],
            &lista->as.list.items[pos],
            (size_t)(lista->as.list.count - pos) * sizeof(LvObject*));
    lista->as.list.items[pos] = args[2];
    lista->as.list.count++;
    return gc_alloc_bool(vm, 1);
}

static LvObject *canon_remover(LvVM *vm, int argc, LvObject **args) {
    if (argc < 2 || !args[0] || args[0]->type != LV_TYPE_LIST)
        return vm->nil_singleton;

    LvObject *lista = args[0];
    for (int i = 0; i < lista->as.list.count; i++) {
        if (!iguais(lista->as.list.items[i], args[1])) continue;
        LvObject *removido = lista->as.list.items[i];
        memmove(&lista->as.list.items[i],
                &lista->as.list.items[i + 1],
                (size_t)(lista->as.list.count - i - 1) * sizeof(LvObject*));
        lista->as.list.count--;
        return removido;
    }
    return vm->nil_singleton;
}

static LvObject *canon_limpar(LvVM *vm, int argc, LvObject **args) {
    if (argc < 1 || !args[0] || args[0]->type != LV_TYPE_LIST)
        return vm->nil_singleton;
    args[0]->as.list.count = 0;
    return args[0];
}

/* ============================================================
 * Entrada legível: teclado.pressionada("A"), etc.
 * ============================================================ */
static int codigo_tecla(LvObject *o, int *ok) {
    *ok = 0;
    if (!o) return 0;

    if (o->type == LV_TYPE_INT) {
        *ok = 1;
        return (int)o->as.i_val;
    }

    if (o->type != LV_TYPE_STRING) return 0;
    const char *s = o->as.string.data;
    if (!s || !*s) return 0;

    if (s[1] == '\0') {
        unsigned char c = (unsigned char)s[0];
        if (isalpha(c)) c = (unsigned char)toupper(c);
        *ok = 1;
        return (int)c;
    }

    if (strcmp(s, "espaco") == 0) { *ok = 1; return LV_KEY_SPACE; }
    if (strcmp(s, "enter") == 0) { *ok = 1; return LV_KEY_ENTER; }
    if (strcmp(s, "esc") == 0) { *ok = 1; return LV_KEY_ESC; }
    if (strcmp(s, "cima") == 0) { *ok = 1; return LV_KEY_UP; }
    if (strcmp(s, "baixo") == 0) { *ok = 1; return LV_KEY_DOWN; }
    if (strcmp(s, "esquerda") == 0) { *ok = 1; return LV_KEY_LEFT; }
    if (strcmp(s, "direita") == 0) { *ok = 1; return LV_KEY_RIGHT; }
    return 0;
}

static LvObject *canon_tecla_down(LvVM *vm, int argc, LvObject **args) {
    int ok = 0;
    int key = argc >= 1 ? codigo_tecla(args[0], &ok) : 0;
    return gc_alloc_bool(vm, ok && game_key_down((LvKeyCode)key));
}

static LvObject *canon_tecla_pressed(LvVM *vm, int argc, LvObject **args) {
    int ok = 0;
    int key = argc >= 1 ? codigo_tecla(args[0], &ok) : 0;
    return gc_alloc_bool(vm, ok && game_key_pressed((LvKeyCode)key));
}

static LvObject *canon_tecla_released(LvVM *vm, int argc, LvObject **args) {
    int ok = 0;
    int key = argc >= 1 ? codigo_tecla(args[0], &ok) : 0;
    return gc_alloc_bool(vm, ok && game_key_released((LvKeyCode)key));
}

static LvObject *canon_mouse_botao(LvVM *vm, int argc, LvObject **args) {
    if (argc < 1 || !args[0]) return gc_alloc_bool(vm, 0);
    int botao = 0;

    if (args[0]->type == LV_TYPE_INT) {
        botao = (int)args[0]->as.i_val;
    } else if (args[0]->type == LV_TYPE_STRING) {
        const char *s = args[0]->as.string.data;
        if (strcmp(s, "esquerdo") == 0) botao = 0;
        else if (strcmp(s, "direito") == 0) botao = 1;
        else if (strcmp(s, "meio") == 0) botao = 2;
        else return gc_alloc_bool(vm, 0);
    } else {
        return gc_alloc_bool(vm, 0);
    }

    return gc_alloc_bool(vm, game_mouse_button_down(botao));
}

/* ============================================================
 * Alias de implementação -> nome canônico
 * ============================================================ */
static void alias_nativa(LvVM *vm, const char *origem, const char *destino) {
    LvObject *obj = hmap_get(&vm->globals, origem);
    if (!obj || obj->type != LV_TYPE_NATIVE) {
        fprintf(stderr,
                "[LEVIATA][Canon] Binding interno ausente: %s -> %s\n",
                origem, destino);
        return;
    }
    vm_register_native(vm, destino, obj->as.native.fn, obj->as.native.arity);
}

static void apagar(LvVM *vm, const char *nome) {
    hmap_delete(&vm->globals, nome);
}

void leviata_canon_register(LvVM *vm) {
    if (!vm) return;

    /* Núcleo de I/O. */
    alias_nativa(vm, "escrever", "escreva");
    alias_nativa(vm, "ler", "leia");

    /* Conversões e introspecção. */
    alias_nativa(vm, "para_texto", "txt");
    alias_nativa(vm, "para_inteiro", "int");
    alias_nativa(vm, "para_decimal", "dec");
    vm_register_native(vm, "bool", canon_bool, 1);
    vm_register_native(vm, "tipo", canon_tipo, 1);

    /* Texto e listas. */
    alias_nativa(vm, "lista_nova", "lista");
    alias_nativa(vm, "lista_adicionar", "adicionar");
    vm_register_native(vm, "contem", canon_contem, 2);
    vm_register_native(vm, "inserir", canon_inserir, 3);
    vm_register_native(vm, "remover", canon_remover, 2);
    vm_register_native(vm, "limpar", canon_limpar, 1);
    alias_nativa(vm, "comeca_com", "comeca");
    alias_nativa(vm, "termina_com", "termina");
    alias_nativa(vm, "fatiar", "trecho");

    /* Matemática: global quando o nome já explica a operação. */
    alias_nativa(vm, "absoluto", "abs");
    alias_nativa(vm, "cosseno", "cos");
    alias_nativa(vm, "tangente", "tan");
    alias_nativa(vm, "logaritmo", "log");
    alias_nativa(vm, "maximo", "max");
    alias_nativa(vm, "minimo", "min");
    alias_nativa(vm, "aleatorio_inteiro", "aleatorio_int");

    /* pi é valor, não chamada. */
    apagar(vm, "pi");
    hmap_set(&vm->globals, "pi", gc_alloc_float(vm, 3.14159265358979323846));

    /* Arquivos: namespace necessário para não confundir leia/escreva de console. */
    alias_nativa(vm, "ler_arquivo", "arquivo.leia");
    alias_nativa(vm, "escrever_arquivo", "arquivo.escreva");
    alias_nativa(vm, "adicionar_arquivo", "arquivo.adicione");

    /* Loop de jogo e desenho. */
    alias_nativa(vm, "jogo_iniciar", "jogo.iniciar");
    alias_nativa(vm, "jogo_fechar", "jogo.fechar");
    alias_nativa(vm, "jogo_deve_fechar", "jogo.deve_fechar");
    alias_nativa(vm, "jogo_iniciar_frame", "jogo.quadro");
    alias_nativa(vm, "jogo_finalizar_frame", "jogo.mostrar");
    alias_nativa(vm, "jogo_delta", "jogo.delta");
    alias_nativa(vm, "jogo_fps", "jogo.fps");

    alias_nativa(vm, "jogo_limpar_fundo", "desenho.fundo");
    alias_nativa(vm, "jogo_desenhar_retangulo", "desenho.retangulo");
    alias_nativa(vm, "jogo_desenhar_circulo", "desenho.circulo");
    alias_nativa(vm, "jogo_desenhar_texto", "desenho.texto");
    alias_nativa(vm, "jogo_desenhar_sprite", "desenho.sprite");
    alias_nativa(vm, "jogo_desenhar_sprite_regiao", "desenho.sprite_regiao");
    alias_nativa(vm, "jogo_desenhar_animacao", "desenho.animacao");
    alias_nativa(vm, "jogo_desenhar_tilemap", "desenho.mapa");

    /* Entrada. */
    vm_register_native(vm, "teclado.pressionada", canon_tecla_down, 1);
    vm_register_native(vm, "teclado.apertada", canon_tecla_pressed, 1);
    vm_register_native(vm, "teclado.solta", canon_tecla_released, 1);
    alias_nativa(vm, "jogo_mouse_x", "mouse.x");
    alias_nativa(vm, "jogo_mouse_y", "mouse.y");
    vm_register_native(vm, "mouse.botao", canon_mouse_botao, 1);

    /* Áudio. */
    alias_nativa(vm, "jogo_audio_iniciar", "audio.iniciar");
    alias_nativa(vm, "jogo_tocar_som", "audio.tocar");
    alias_nativa(vm, "jogo_tocar_musica", "audio.musica");
    alias_nativa(vm, "jogo_volume", "audio.volume");

    /* Câmera e colisão. */
    alias_nativa(vm, "jogo_camera_configurar", "camera.configurar");
    alias_nativa(vm, "jogo_camera_seguir", "camera.seguir");
    alias_nativa(vm, "jogo_camera_suavidade", "camera.suavidade");
    alias_nativa(vm, "jogo_camera_iniciar", "camera.iniciar");
    alias_nativa(vm, "jogo_camera_finalizar", "camera.finalizar");
    alias_nativa(vm, "jogo_camera_limites", "camera.limites");
    alias_nativa(vm, "jogo_camera_limites_limpar", "camera.limpar_limites");

    alias_nativa(vm, "jogo_colisao_retangulos", "colisao.retangulos");
    alias_nativa(vm, "jogo_ponto_no_retangulo", "colisao.ponto_retangulo");
    alias_nativa(vm, "jogo_colisao_circulos", "colisao.circulos");
    alias_nativa(vm, "jogo_colisao_tilemap", "colisao.mapa");
    alias_nativa(vm, "jogo_animacao_frame", "animacao.frame");

    /* Cenas. */
    alias_nativa(vm, "cena_mudar", "cena.mudar");
    alias_nativa(vm, "cena_atual", "cena.atual");
    alias_nativa(vm, "cena_anterior", "cena.anterior");
    alias_nativa(vm, "cena_e", "cena.ativa");
    alias_nativa(vm, "cena_versao", "cena.versao");

    /* Entidades. */
    alias_nativa(vm, "entidade_criar", "entidade.criar");
    alias_nativa(vm, "entidade_destruir", "entidade.destruir");
    alias_nativa(vm, "entidade_buscar", "entidade.buscar");
    alias_nativa(vm, "entidade_posicionar", "entidade.posicionar");
    alias_nativa(vm, "entidade_tamanho", "entidade.tamanho");
    alias_nativa(vm, "entidade_velocidade", "entidade.velocidade");
    alias_nativa(vm, "entidade_fisica", "entidade.fisica");
    alias_nativa(vm, "entidade_colisor", "entidade.colisor");
    alias_nativa(vm, "entidade_gatilho", "entidade.gatilho");
    alias_nativa(vm, "entidade_colide", "entidade.colide");
    alias_nativa(vm, "entidade_distancia", "entidade.distancia");
    alias_nativa(vm, "entidade_atualizar", "entidade.atualizar");
    alias_nativa(vm, "entidade_atualizar_tilemap", "entidade.atualizar_mapa");
    alias_nativa(vm, "entidade_sprite", "entidade.sprite");
    alias_nativa(vm, "entidade_sprite_remover", "entidade.remover_sprite");
    alias_nativa(vm, "entidade_sprite_inverter_x", "entidade.inverter_sprite_x");
    alias_nativa(vm, "entidade_animacao_reproduzir", "entidade.animar");
    alias_nativa(vm, "entidade_animacao_frame", "entidade.frame");
    alias_nativa(vm, "entidade_desenhar_sprite", "entidade.desenhar_sprite");
    alias_nativa(vm, "entidade_desenhar", "entidade.desenhar");
    alias_nativa(vm, "entidade_x", "entidade.x");
    alias_nativa(vm, "entidade_y", "entidade.y");
    alias_nativa(vm, "entidade_vx", "entidade.vx");
    alias_nativa(vm, "entidade_vy", "entidade.vy");
    alias_nativa(vm, "entidade_no_chao", "entidade.no_chao");
    alias_nativa(vm, "entidade_nome", "entidade.nome");
    alias_nativa(vm, "entidade_tem_componente", "entidade.tem");
    alias_nativa(vm, "entidade_quantidade", "entidade.quantidade");

    /* Mapas. */
    alias_nativa(vm, "mapa_carregar", "mapa.carregar");
    alias_nativa(vm, "mapa_largura", "mapa.largura");
    alias_nativa(vm, "mapa_altura", "mapa.altura");

    /* Remove a superfície antiga. Implementações C continuam reutilizadas. */
    static const char *antigos[] = {
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

    for (int i = 0; antigos[i] != NULL; i++)
        apagar(vm, antigos[i]);
}
