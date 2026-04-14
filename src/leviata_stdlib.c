/* ============================================================================
 * MOTOR LEVIATA - BIBLIOTECA PADRÃO (STD LIBRARY)
 * Todas as funções nativas disponíveis no ambiente PT.
 * ============================================================================ */
#include "leviata_stdlib.h"
#include "leviata_ffi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <ctype.h>

/* Macro de conveniência para declarar funções nativas */
#define NATIVE_FN(nome) \
    static LvObject *native_##nome(LvVM *vm, int argc, LvObject **args)

/* Verificações de argumento */
#define REQUIRE_ARGC(n) \
    do { if (argc < (n)) { \
        fprintf(stderr, "[STD] Função espera %d argumento(s), recebeu %d\n", (n), argc); \
        return vm->nil_singleton; \
    }} while(0)

#define ARG_INT(i)   ((args[i])->as.i_val)
#define ARG_FLOAT(i) ((args[i])->type == LV_TYPE_FLOAT ? (args[i])->as.f_val : (double)(args[i])->as.i_val)
#define ARG_STR(i)   ((args[i])->as.string.data)

/* ============================================================
 * MÓDULO I/O — Entrada, Saída e Arquivos
 * ============================================================ */

NATIVE_FN(escrever) {
    for (int i = 0; i < argc; i++) {
        LvObject *o = args[i];
        if (!o) { printf("nulo"); continue; }
        switch (o->type) {
            case LV_TYPE_NULL:   printf("nulo"); break;
            case LV_TYPE_BOOL:   printf("%s", o->as.i_val ? "verdadeiro" : "falso"); break;
            case LV_TYPE_INT:    printf("%lld", (long long)o->as.i_val); break;
            case LV_TYPE_FLOAT:  printf("%g", o->as.f_val); break;
            case LV_TYPE_STRING: printf("%s", o->as.string.data); break;
            case LV_TYPE_LIST:   printf("[Lista:%d]", o->as.list.count); break;
            case LV_TYPE_NATIVE: printf("<Nativa:%s>", o->as.native.name); break;
            default:             printf("<objeto>"); break;
        }
        if (i < argc - 1) printf(" ");
    }
    printf("\n");
    return vm->nil_singleton;
}

NATIVE_FN(ler) {
    char buf[LV_MAX_STRING];
    if (argc > 0 && args[0]->type == LV_TYPE_STRING)
        printf("%s", args[0]->as.string.data);
    if (!fgets(buf, sizeof(buf), stdin))
        return vm->nil_singleton;
    size_t len = strlen(buf);
    if (len > 0 && buf[len-1] == '\n') buf[--len] = '\0';
    return gc_alloc_string(vm, buf, len);
}

NATIVE_FN(ler_arquivo) {
    REQUIRE_ARGC(1);
    const char *path = ARG_STR(0);
    FILE *f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "[STD] Não foi possível abrir arquivo: %s\n", path);
        return vm->nil_singleton;
    }
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);
    char *buf = (char*)malloc(size + 1);
    fread(buf, 1, size, f);
    buf[size] = '\0';
    fclose(f);
    LvObject *s = gc_alloc_string(vm, buf, (size_t)size);
    free(buf);
    return s;
}

NATIVE_FN(escrever_arquivo) {
    REQUIRE_ARGC(2);
    const char *path    = ARG_STR(0);
    const char *content = ARG_STR(1);
    FILE *f = fopen(path, "w");
    if (!f) {
        fprintf(stderr, "[STD] Não foi possível escrever arquivo: %s\n", path);
        return gc_alloc_bool(vm, 0);
    }
    fputs(content, f);
    fclose(f);
    return gc_alloc_bool(vm, 1);
}

NATIVE_FN(adicionar_arquivo) {
    REQUIRE_ARGC(2);
    FILE *f = fopen(ARG_STR(0), "a");
    if (!f) return gc_alloc_bool(vm, 0);
    fputs(ARG_STR(1), f);
    fclose(f);
    return gc_alloc_bool(vm, 1);
}

void stdlib_register_io(LvVM *vm) {
    vm_register_native(vm, "escrever",       native_escrever,       -1);
    vm_register_native(vm, "imprimir",       native_escrever,       -1); /* alias */
    vm_register_native(vm, "ler",            native_ler,            -1);
    vm_register_native(vm, "ler_arquivo",    native_ler_arquivo,     1);
    vm_register_native(vm, "escrever_arquivo", native_escrever_arquivo, 2);
    vm_register_native(vm, "adicionar_arquivo", native_adicionar_arquivo, 2);
}

/* ============================================================
 * MÓDULO MATH — Matemática Básica e Avançada
 * ============================================================ */

NATIVE_FN(raiz) {
    REQUIRE_ARGC(1);
    return gc_alloc_float(vm, sqrt(ARG_FLOAT(0)));
}

NATIVE_FN(potencia) {
    REQUIRE_ARGC(2);
    double base = ARG_FLOAT(0), exp = ARG_FLOAT(1);
    double r = pow(base, exp);
    if (r == (int64_t)r) return gc_alloc_int(vm, (int64_t)r);
    return gc_alloc_float(vm, r);
}

NATIVE_FN(absoluto) {
    REQUIRE_ARGC(1);
    if (args[0]->type == LV_TYPE_INT)
        return gc_alloc_int(vm, llabs(ARG_INT(0)));
    return gc_alloc_float(vm, fabs(ARG_FLOAT(0)));
}

NATIVE_FN(arredondar) {
    REQUIRE_ARGC(1);
    return gc_alloc_int(vm, (int64_t)round(ARG_FLOAT(0)));
}

NATIVE_FN(teto) {
    REQUIRE_ARGC(1);
    return gc_alloc_int(vm, (int64_t)ceil(ARG_FLOAT(0)));
}

NATIVE_FN(piso) {
    REQUIRE_ARGC(1);
    return gc_alloc_int(vm, (int64_t)floor(ARG_FLOAT(0)));
}

NATIVE_FN(seno) {
    REQUIRE_ARGC(1);
    return gc_alloc_float(vm, sin(ARG_FLOAT(0)));
}

NATIVE_FN(cosseno) {
    REQUIRE_ARGC(1);
    return gc_alloc_float(vm, cos(ARG_FLOAT(0)));
}

NATIVE_FN(tangente) {
    REQUIRE_ARGC(1);
    return gc_alloc_float(vm, tan(ARG_FLOAT(0)));
}

NATIVE_FN(logaritmo) {
    REQUIRE_ARGC(1);
    return gc_alloc_float(vm, log(ARG_FLOAT(0)));
}

NATIVE_FN(logaritmo10) {
    REQUIRE_ARGC(1);
    return gc_alloc_float(vm, log10(ARG_FLOAT(0)));
}

NATIVE_FN(aleatorio) {
    /* Retorna float entre 0.0 e 1.0 */
    return gc_alloc_float(vm, (double)rand() / RAND_MAX);
}

NATIVE_FN(aleatorio_inteiro) {
    REQUIRE_ARGC(2);
    int64_t lo = ARG_INT(0), hi = ARG_INT(1);
    if (hi <= lo) return gc_alloc_int(vm, lo);
    return gc_alloc_int(vm, lo + (rand() % (hi - lo + 1)));
}

NATIVE_FN(maximo) {
    REQUIRE_ARGC(2);
    double a = ARG_FLOAT(0), b = ARG_FLOAT(1);
    if (args[0]->type == LV_TYPE_INT && args[1]->type == LV_TYPE_INT)
        return gc_alloc_int(vm, ARG_INT(0) > ARG_INT(1) ? ARG_INT(0) : ARG_INT(1));
    return gc_alloc_float(vm, a > b ? a : b);
}

NATIVE_FN(minimo) {
    REQUIRE_ARGC(2);
    double a = ARG_FLOAT(0), b = ARG_FLOAT(1);
    if (args[0]->type == LV_TYPE_INT && args[1]->type == LV_TYPE_INT)
        return gc_alloc_int(vm, ARG_INT(0) < ARG_INT(1) ? ARG_INT(0) : ARG_INT(1));
    return gc_alloc_float(vm, a < b ? a : b);
}

NATIVE_FN(pi) {
    (void)argc; (void)args;
    return gc_alloc_float(vm, 3.14159265358979323846);
}

void stdlib_register_math(LvVM *vm) {
    srand((unsigned)time(NULL));
    vm_register_native(vm, "raiz",            native_raiz,            1);
    vm_register_native(vm, "potencia",        native_potencia,        2);
    vm_register_native(vm, "absoluto",        native_absoluto,        1);
    vm_register_native(vm, "arredondar",      native_arredondar,      1);
    vm_register_native(vm, "teto",            native_teto,            1);
    vm_register_native(vm, "piso",            native_piso,            1);
    vm_register_native(vm, "seno",            native_seno,            1);
    vm_register_native(vm, "cosseno",         native_cosseno,         1);
    vm_register_native(vm, "tangente",        native_tangente,        1);
    vm_register_native(vm, "logaritmo",       native_logaritmo,       1);
    vm_register_native(vm, "logaritmo10",     native_logaritmo10,     1);
    vm_register_native(vm, "aleatorio",       native_aleatorio,       0);
    vm_register_native(vm, "aleatorio_inteiro", native_aleatorio_inteiro, 2);
    vm_register_native(vm, "maximo",          native_maximo,          2);
    vm_register_native(vm, "minimo",          native_minimo,          2);
    vm_register_native(vm, "pi",              native_pi,              0);
}

/* ============================================================
 * MÓDULO STRING — Manipulação de Texto
 * ============================================================ */

NATIVE_FN(tamanho) {
    REQUIRE_ARGC(1);
    if (args[0]->type == LV_TYPE_STRING)
        return gc_alloc_int(vm, (int64_t)args[0]->as.string.length);
    if (args[0]->type == LV_TYPE_LIST)
        return gc_alloc_int(vm, (int64_t)args[0]->as.list.count);
    return gc_alloc_int(vm, 0);
}

NATIVE_FN(para_texto) {
    REQUIRE_ARGC(1);
    char buf[64];
    LvObject *o = args[0];
    switch (o->type) {
        case LV_TYPE_NULL:   return gc_alloc_string(vm, "nulo", 4);
        case LV_TYPE_BOOL:   return gc_alloc_string(vm, o->as.i_val ? "verdadeiro" : "falso",
                                                    o->as.i_val ? 10 : 5);
        case LV_TYPE_INT:    snprintf(buf, sizeof(buf), "%lld", (long long)o->as.i_val);
                             return gc_alloc_string(vm, buf, strlen(buf));
        case LV_TYPE_FLOAT:  snprintf(buf, sizeof(buf), "%g", o->as.f_val);
                             return gc_alloc_string(vm, buf, strlen(buf));
        case LV_TYPE_STRING: return o;
        default:             return gc_alloc_string(vm, "<objeto>", 8);
    }
}

NATIVE_FN(para_inteiro) {
    REQUIRE_ARGC(1);
    if (args[0]->type == LV_TYPE_STRING)
        return gc_alloc_int(vm, (int64_t)atoll(args[0]->as.string.data));
    if (args[0]->type == LV_TYPE_FLOAT)
        return gc_alloc_int(vm, (int64_t)args[0]->as.f_val);
    if (args[0]->type == LV_TYPE_INT)
        return args[0];
    return gc_alloc_int(vm, 0);
}

NATIVE_FN(para_decimal) {
    REQUIRE_ARGC(1);
    if (args[0]->type == LV_TYPE_STRING)
        return gc_alloc_float(vm, atof(args[0]->as.string.data));
    if (args[0]->type == LV_TYPE_INT)
        return gc_alloc_float(vm, (double)args[0]->as.i_val);
    if (args[0]->type == LV_TYPE_FLOAT)
        return args[0];
    return gc_alloc_float(vm, 0.0);
}

NATIVE_FN(maiusculas) {
    REQUIRE_ARGC(1);
    if (args[0]->type != LV_TYPE_STRING) return vm->nil_singleton;
    char *src = args[0]->as.string.data;
    size_t len = args[0]->as.string.length;
    char *buf = (char*)malloc(len + 1);
    for (size_t i = 0; i < len; i++) buf[i] = toupper((unsigned char)src[i]);
    buf[len] = '\0';
    LvObject *s = gc_alloc_string(vm, buf, len);
    free(buf);
    return s;
}

NATIVE_FN(minusculas) {
    REQUIRE_ARGC(1);
    if (args[0]->type != LV_TYPE_STRING) return vm->nil_singleton;
    char *src = args[0]->as.string.data;
    size_t len = args[0]->as.string.length;
    char *buf = (char*)malloc(len + 1);
    for (size_t i = 0; i < len; i++) buf[i] = tolower((unsigned char)src[i]);
    buf[len] = '\0';
    LvObject *s = gc_alloc_string(vm, buf, len);
    free(buf);
    return s;
}

NATIVE_FN(contem) {
    REQUIRE_ARGC(2);
    if (args[0]->type != LV_TYPE_STRING || args[1]->type != LV_TYPE_STRING)
        return gc_alloc_bool(vm, 0);
    return gc_alloc_bool(vm, strstr(ARG_STR(0), ARG_STR(1)) != NULL);
}

NATIVE_FN(começa_com) {
    REQUIRE_ARGC(2);
    if (args[0]->type != LV_TYPE_STRING || args[1]->type != LV_TYPE_STRING)
        return gc_alloc_bool(vm, 0);
    return gc_alloc_bool(vm, strncmp(ARG_STR(0), ARG_STR(1), strlen(ARG_STR(1))) == 0);
}

NATIVE_FN(termina_com) {
    REQUIRE_ARGC(2);
    if (args[0]->type != LV_TYPE_STRING || args[1]->type != LV_TYPE_STRING)
        return gc_alloc_bool(vm, 0);
    size_t la = strlen(ARG_STR(0)), lb = strlen(ARG_STR(1));
    if (lb > la) return gc_alloc_bool(vm, 0);
    return gc_alloc_bool(vm, strcmp(ARG_STR(0) + la - lb, ARG_STR(1)) == 0);
}

NATIVE_FN(fatiar) {
    REQUIRE_ARGC(3);
    if (args[0]->type != LV_TYPE_STRING) return vm->nil_singleton;
    const char *s = ARG_STR(0);
    size_t len = strlen(s);
    int64_t ini = ARG_INT(1), fim = ARG_INT(2);
    if (ini < 0) ini = 0;
    if (fim > (int64_t)len) fim = (int64_t)len;
    if (ini >= fim) return gc_alloc_string(vm, "", 0);
    return gc_alloc_string(vm, s + ini, (size_t)(fim - ini));
}

void stdlib_register_string(LvVM *vm) {
    vm_register_native(vm, "tamanho",       native_tamanho,      1);
    vm_register_native(vm, "para_texto",    native_para_texto,   1);
    vm_register_native(vm, "para_inteiro",  native_para_inteiro, 1);
    vm_register_native(vm, "para_decimal",  native_para_decimal, 1);
    vm_register_native(vm, "maiusculas",    native_maiusculas,   1);
    vm_register_native(vm, "minusculas",    native_minusculas,   1);
    vm_register_native(vm, "contem",        native_contem,       2);
    vm_register_native(vm, "comeca_com",    native_começa_com,   2);
    vm_register_native(vm, "termina_com",   native_termina_com,  2);
    vm_register_native(vm, "fatiar",        native_fatiar,       3);
}

/* ============================================================
 * MÓDULO LISTA — Arrays Dinâmicos
 * ============================================================ */

NATIVE_FN(lista_nova) {
    (void)argc; (void)args;
    return gc_alloc_list(vm);
}

NATIVE_FN(lista_adicionar) {
    REQUIRE_ARGC(2);
    LvObject *lst = args[0];
    if (lst->type != LV_TYPE_LIST) {
        fprintf(stderr, "[STD] lista_adicionar: primeiro argumento deve ser lista\n");
        return vm->nil_singleton;
    }
    if (lst->as.list.count >= lst->as.list.capacity) {
        lst->as.list.capacity *= 2;
        lst->as.list.items = (LvObject**)realloc(lst->as.list.items,
                                                  lst->as.list.capacity * sizeof(LvObject*));
    }
    lst->as.list.items[lst->as.list.count++] = args[1];
    return vm->nil_singleton;
}

NATIVE_FN(lista_remover) {
    REQUIRE_ARGC(2);
    LvObject *lst = args[0];
    if (lst->type != LV_TYPE_LIST) return vm->nil_singleton;
    int64_t idx = ARG_INT(1);
    if (idx < 0 || idx >= lst->as.list.count) return vm->nil_singleton;
    LvObject *removed = lst->as.list.items[idx];
    memmove(&lst->as.list.items[idx], &lst->as.list.items[idx+1],
            (lst->as.list.count - idx - 1) * sizeof(LvObject*));
    lst->as.list.count--;
    return removed;
}

NATIVE_FN(lista_contem) {
    REQUIRE_ARGC(2);
    LvObject *lst = args[0];
    if (lst->type != LV_TYPE_LIST) return gc_alloc_bool(vm, 0);
    for (int i = 0; i < lst->as.list.count; i++) {
        LvObject *a = lst->as.list.items[i], *b = args[1];
        if (a->type == b->type) {
            if (a->type == LV_TYPE_INT && a->as.i_val == b->as.i_val) return gc_alloc_bool(vm, 1);
            if (a->type == LV_TYPE_FLOAT && a->as.f_val == b->as.f_val) return gc_alloc_bool(vm, 1);
            if (a->type == LV_TYPE_STRING && strcmp(a->as.string.data, b->as.string.data) == 0)
                return gc_alloc_bool(vm, 1);
        }
    }
    return gc_alloc_bool(vm, 0);
}

void stdlib_register_list(LvVM *vm) {
    vm_register_native(vm, "lista_nova",      native_lista_nova,      0);
    vm_register_native(vm, "lista_adicionar", native_lista_adicionar, 2);
    vm_register_native(vm, "lista_remover",   native_lista_remover,   2);
    vm_register_native(vm, "lista_contem",    native_lista_contem,    2);
}

/* ============================================================
 * MÓDULO SISTEMA — Tempo, OS
 * ============================================================ */

NATIVE_FN(tempo_atual) {
    (void)argc; (void)args;
    return gc_alloc_float(vm, (double)clock() / CLOCKS_PER_SEC);
}

NATIVE_FN(dormir) {
    REQUIRE_ARGC(1);
    double secs = ARG_FLOAT(0);
    (void)secs;
    /* Portável sem dependência POSIX pura */
    clock_t start = clock();
    clock_t wait  = (clock_t)(secs * CLOCKS_PER_SEC);
    while (clock() - start < wait) { /* busy-wait para portabilidade */ }
    return vm->nil_singleton;
}

NATIVE_FN(sair) {
    int code = (argc > 0 && args[0]->type == LV_TYPE_INT) ? (int)ARG_INT(0) : 0;
    exit(code);
    return vm->nil_singleton;
}

void stdlib_register_system(LvVM *vm) {
    vm_register_native(vm, "tempo_atual",  native_tempo_atual,  0);
    vm_register_native(vm, "dormir",       native_dormir,       1);
    vm_register_native(vm, "sair",         native_sair,        -1);
}

/* ============================================================
 * MÓDULO GRÁFICOS — FFI / Bindings de UI
 * Stub preparado para integração real com SDL2/GTK/Win32.
 * ============================================================ */

NATIVE_FN(janela_criar) {
    printf("[UI-FFI] janela_criar chamada");
    if (argc >= 3) {
        printf(" - titulo='%s' %ldx%ld",
               ARG_STR(0), (long)ARG_INT(1), (long)ARG_INT(2));
    }
    printf("\n");
    printf("[UI-FFI] ** Para ativar UI real, compile com -DLVA_USE_SDL2 e link -lSDL2 **\n");
    return vm->nil_singleton;
}

NATIVE_FN(janela_fechar) {
    printf("[UI-FFI] janela_fechar chamada\n");
    return vm->nil_singleton;
}

NATIVE_FN(desenhar_retangulo) {
    if (argc >= 4)
        printf("[UI-FFI] desenhar_retangulo x=%.0f y=%.0f w=%.0f h=%.0f\n",
               ARG_FLOAT(0), ARG_FLOAT(1), ARG_FLOAT(2), ARG_FLOAT(3));
    return vm->nil_singleton;
}

NATIVE_FN(desenhar_texto) {
    if (argc >= 3)
        printf("[UI-FFI] desenhar_texto '%s' em (%.0f, %.0f)\n",
               ARG_STR(0), ARG_FLOAT(1), ARG_FLOAT(2));
    return vm->nil_singleton;
}

void stdlib_register_graphics(LvVM *vm) {
    vm_register_native(vm, "janela_criar",         native_janela_criar,       -1);
    vm_register_native(vm, "janela_fechar",        native_janela_fechar,       0);
    vm_register_native(vm, "desenhar_retangulo",   native_desenhar_retangulo, -1);
    vm_register_native(vm, "desenhar_texto",       native_desenhar_texto,     -1);
}

/* ============================================================
 * MÓDULO JOGO — Game Engine Bindings (Raylib/SDL stub)
 * ============================================================ */

NATIVE_FN(jogo_iniciar) {
    printf("[GAME-FFI] jogo_iniciar chamado");
    if (argc >= 3)
        printf(" - titulo='%s' %ldx%ld", ARG_STR(0), (long)ARG_INT(1), (long)ARG_INT(2));
    printf("\n");
    printf("[GAME-FFI] ** Game Loop stub ativo **\n");
    printf("[GAME-FFI] ** Para Raylib real: compile com -DLVA_USE_RAYLIB e link -lraylib **\n");
    return vm->nil_singleton;
}

NATIVE_FN(jogo_fechar) {
    printf("[GAME-FFI] jogo_fechar chamado - recursos liberados\n");
    return vm->nil_singleton;
}

NATIVE_FN(jogo_deve_fechar) {
    /* Em modo stub, nunca fecha automaticamente */
    return gc_alloc_bool(vm, 0);
}

NATIVE_FN(jogo_iniciar_frame) {
    printf("[GAME-FFI] --- Início do Frame ---\n");
    return vm->nil_singleton;
}

NATIVE_FN(jogo_finalizar_frame) {
    printf("[GAME-FFI] --- Fim do Frame (swap buffer) ---\n");
    return vm->nil_singleton;
}

NATIVE_FN(jogo_limpar_fundo) {
    if (argc >= 3)
        printf("[GAME-FFI] limpar_fundo RGB(%lld,%lld,%lld)\n",
               (long long)ARG_INT(0), (long long)ARG_INT(1), (long long)ARG_INT(2));
    return vm->nil_singleton;
}

NATIVE_FN(jogo_desenhar_sprite) {
    if (argc >= 3)
        printf("[GAME-FFI] desenhar_sprite '%s' em (%.1f, %.1f)\n",
               ARG_STR(0), ARG_FLOAT(1), ARG_FLOAT(2));
    return vm->nil_singleton;
}

NATIVE_FN(jogo_tecla_pressionada) {
    REQUIRE_ARGC(1);
    printf("[GAME-FFI] verificar_tecla(%lld) - retorna falso em modo stub\n",
           (long long)ARG_INT(0));
    return gc_alloc_bool(vm, 0);
}

NATIVE_FN(jogo_mouse_x) {
    (void)argc; (void)args;
    return gc_alloc_float(vm, 0.0);
}

NATIVE_FN(jogo_mouse_y) {
    (void)argc; (void)args;
    return gc_alloc_float(vm, 0.0);
}

NATIVE_FN(jogo_delta_tempo) {
    (void)argc; (void)args;
    return gc_alloc_float(vm, 1.0 / 60.0); /* Stub: 60fps fixo */
}

void stdlib_register_game(LvVM *vm) {
    vm_register_native(vm, "jogo_iniciar",         native_jogo_iniciar,        -1);
    vm_register_native(vm, "jogo_fechar",          native_jogo_fechar,          0);
    vm_register_native(vm, "jogo_deve_fechar",     native_jogo_deve_fechar,     0);
    vm_register_native(vm, "jogo_iniciar_frame",   native_jogo_iniciar_frame,   0);
    vm_register_native(vm, "jogo_finalizar_frame", native_jogo_finalizar_frame, 0);
    vm_register_native(vm, "jogo_limpar_fundo",    native_jogo_limpar_fundo,   -1);
    vm_register_native(vm, "jogo_desenhar_sprite", native_jogo_desenhar_sprite,-1);
    vm_register_native(vm, "jogo_tecla",           native_jogo_tecla_pressionada, 1);
    vm_register_native(vm, "jogo_mouse_x",         native_jogo_mouse_x,         0);
    vm_register_native(vm, "jogo_mouse_y",         native_jogo_mouse_y,         0);
    vm_register_native(vm, "jogo_delta",           native_jogo_delta_tempo,     0);
}

/* ============================================================
 * REGISTRAR TUDO
 * ============================================================ */
void stdlib_register_all(LvVM *vm) {
    stdlib_register_io(vm);
    stdlib_register_math(vm);
    stdlib_register_string(vm);
    stdlib_register_list(vm);
    stdlib_register_system(vm);
    stdlib_register_graphics(vm);
    stdlib_register_game(vm);
}
