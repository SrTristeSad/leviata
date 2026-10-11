/* ============================================================================
 * LEVIATA 3.1 - OBJETOS NATIVOS DE RECURSO
 *
 * A sintaxe de método é compilada para handlers internos impossíveis de
 * escrever diretamente no código Leviata (nomes iniciados por '@').
 * ============================================================================ */
#include "leviata_resources.h"
#include "leviata_ffi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static double numero(LvObject *o) {
    if (!o) return 0.0;
    if (o->type == LV_TYPE_FLOAT) return o->as.f_val;
    if (o->type == LV_TYPE_INT || o->type == LV_TYPE_BOOL)
        return (double)o->as.i_val;
    return 0.0;
}

static LvNativeObjectData *recurso(LvObject *o) {
    if (!o || o->type != LV_TYPE_OBJECT) return NULL;
    return &o->as.object;
}

static LvNativeObjectData *recurso_tipo(LvObject *o, LvObjectKind kind) {
    LvNativeObjectData *r = recurso(o);
    if (!r || r->kind != kind) return NULL;
    return r;
}

static LvObject *novo_recurso(LvVM *vm, LvObjectKind kind, const char *path) {
    LvObject *o = gc_alloc_object(vm, LV_TYPE_OBJECT);
    o->as.object.kind = kind;
    o->as.object.volume = 1.0;
    o->as.object.width = 0;
    o->as.object.height = 0;
    strncpy(o->as.object.path, path ? path : "", LV_RESOURCE_PATH_MAX - 1);
    o->as.object.path[LV_RESOURCE_PATH_MAX - 1] = '\0';
    return o;
}

static const char *nome_tipo_objeto(LvObjectKind kind) {
    switch (kind) {
        case LV_OBJECT_IMAGE: return "imagem";
        case LV_OBJECT_SOUND: return "som";
        case LV_OBJECT_FILE: return "arquivo";
        default: return "objeto";
    }
}

/* ============================================================
 * tipo()
 * ============================================================ */
static LvObject *resource_tipo(LvVM *vm, int argc, LvObject **args) {
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
            case LV_TYPE_OBJECT:
                nome = nome_tipo_objeto(args[0]->as.object.kind);
                break;
            default: nome = "objeto"; break;
        }
    }
    return gc_alloc_string(vm, nome, strlen(nome));
}

/* ============================================================
 * Construtores
 * ============================================================ */
static LvObject *resource_imagem(LvVM *vm, int argc, LvObject **args) {
    if (argc < 1 || !args[0] || args[0]->type != LV_TYPE_STRING)
        return vm->nil_singleton;
    return novo_recurso(vm, LV_OBJECT_IMAGE, args[0]->as.string.data);
}

static LvObject *resource_som(LvVM *vm, int argc, LvObject **args) {
    if (argc < 1 || !args[0] || args[0]->type != LV_TYPE_STRING)
        return vm->nil_singleton;
    return novo_recurso(vm, LV_OBJECT_SOUND, args[0]->as.string.data);
}

static LvObject *resource_arquivo(LvVM *vm, int argc, LvObject **args) {
    if (argc < 1 || !args[0] || args[0]->type != LV_TYPE_STRING)
        return vm->nil_singleton;
    return novo_recurso(vm, LV_OBJECT_FILE, args[0]->as.string.data);
}

/* ============================================================
 * Campos
 * ============================================================ */
static int atualizar_dimensoes_imagem(LvNativeObjectData *r) {
    if (!r || r->kind != LV_OBJECT_IMAGE) return 0;
    if (r->width > 0 && r->height > 0) return 1;

    LvTexture *tex = game_load_texture(r->path);
    if (!tex) return 0;
    r->width = tex->width;
    r->height = tex->height;
    game_unload_texture(tex);
    return r->width > 0 && r->height > 0;
}

static LvObject *field_caminho(LvVM *vm, int argc, LvObject **args) {
    LvNativeObjectData *r = argc >= 1 ? recurso(args[0]) : NULL;
    if (!r) return vm->nil_singleton;
    return gc_alloc_string(vm, r->path, strlen(r->path));
}

static LvObject *field_largura(LvVM *vm, int argc, LvObject **args) {
    LvNativeObjectData *r = argc >= 1 ? recurso_tipo(args[0], LV_OBJECT_IMAGE) : NULL;
    if (!r) return vm->nil_singleton;
    atualizar_dimensoes_imagem(r);
    return gc_alloc_int(vm, r->width);
}

static LvObject *field_altura(LvVM *vm, int argc, LvObject **args) {
    LvNativeObjectData *r = argc >= 1 ? recurso_tipo(args[0], LV_OBJECT_IMAGE) : NULL;
    if (!r) return vm->nil_singleton;
    atualizar_dimensoes_imagem(r);
    return gc_alloc_int(vm, r->height);
}

static LvObject *field_volume(LvVM *vm, int argc, LvObject **args) {
    LvNativeObjectData *r = argc >= 1 ? recurso_tipo(args[0], LV_OBJECT_SOUND) : NULL;
    if (!r) return vm->nil_singleton;
    return gc_alloc_float(vm, r->volume);
}

/* ============================================================
 * Métodos de imagem
 * ============================================================ */
static LvObject *method_desenhe(LvVM *vm, int argc, LvObject **args) {
    LvNativeObjectData *r = argc >= 1 ? recurso_tipo(args[0], LV_OBJECT_IMAGE) : NULL;
    if (!r || argc < 3) return gc_alloc_bool(vm, 0);

    LvObject *desenhar = hmap_get(&vm->globals, "desenho.sprite");
    if (!desenhar || desenhar->type != LV_TYPE_NATIVE)
        return gc_alloc_bool(vm, 0);

    LvObject *path = gc_alloc_string(vm, r->path, strlen(r->path));
    LvObject *call_args[5];
    int call_argc = 3;
    call_args[0] = path;
    call_args[1] = args[1];
    call_args[2] = args[2];

    if (argc >= 4) {
        call_args[3] = args[3];
        call_argc = 4;
    }
    if (argc >= 5) {
        call_args[4] = args[4];
        call_argc = 5;
    }

    LvObject *result = desenhar->as.native.fn(vm, call_argc, call_args);
    return result ? result : vm->nil_singleton;
}

/* ============================================================
 * Métodos de som
 * ============================================================ */
static LvObject *method_toque(LvVM *vm, int argc, LvObject **args) {
    LvNativeObjectData *r = argc >= 1 ? recurso_tipo(args[0], LV_OBJECT_SOUND) : NULL;
    if (!r) return gc_alloc_bool(vm, 0);

    /* O backend atual possui volume mestre; o objeto preserva a intenção e
     * aplica seu volume imediatamente antes da reprodução. */
    game_set_volume((float)r->volume);
    game_play_sound(r->path);
    return vm->nil_singleton;
}

static LvObject *method_volume(LvVM *vm, int argc, LvObject **args) {
    LvNativeObjectData *r = argc >= 1 ? recurso_tipo(args[0], LV_OBJECT_SOUND) : NULL;
    if (!r) return vm->nil_singleton;

    if (argc >= 2) {
        double v = numero(args[1]);
        if (v < 0.0) v = 0.0;
        if (v > 1.0) v = 1.0;
        r->volume = v;
    }

    return gc_alloc_float(vm, r->volume);
}

/* ============================================================
 * Métodos de arquivo
 * ============================================================ */
static LvObject *method_leia(LvVM *vm, int argc, LvObject **args) {
    LvNativeObjectData *r = argc >= 1 ? recurso_tipo(args[0], LV_OBJECT_FILE) : NULL;
    if (!r) return vm->nil_singleton;

    FILE *f = fopen(r->path, "rb");
    if (!f) return vm->nil_singleton;

    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return vm->nil_singleton;
    }
    long size = ftell(f);
    if (size < 0) {
        fclose(f);
        return vm->nil_singleton;
    }
    rewind(f);

    char *buf = (char*)malloc((size_t)size + 1);
    if (!buf) {
        fclose(f);
        return vm->nil_singleton;
    }

    size_t lidos = fread(buf, 1, (size_t)size, f);
    buf[lidos] = '\0';
    fclose(f);

    LvObject *texto = gc_alloc_string(vm, buf, lidos);
    free(buf);
    return texto;
}

static LvObject *method_escreva(LvVM *vm, int argc, LvObject **args) {
    LvNativeObjectData *r = argc >= 1 ? recurso_tipo(args[0], LV_OBJECT_FILE) : NULL;
    if (!r || argc < 2 || !args[1] || args[1]->type != LV_TYPE_STRING)
        return gc_alloc_bool(vm, 0);

    FILE *f = fopen(r->path, "wb");
    if (!f) return gc_alloc_bool(vm, 0);
    size_t len = args[1]->as.string.length;
    size_t escritos = fwrite(args[1]->as.string.data, 1, len, f);
    fclose(f);
    return gc_alloc_bool(vm, escritos == len);
}

static LvObject *method_adicione(LvVM *vm, int argc, LvObject **args) {
    LvNativeObjectData *r = argc >= 1 ? recurso_tipo(args[0], LV_OBJECT_FILE) : NULL;
    if (!r || argc < 2 || !args[1] || args[1]->type != LV_TYPE_STRING)
        return gc_alloc_bool(vm, 0);

    FILE *f = fopen(r->path, "ab");
    if (!f) return gc_alloc_bool(vm, 0);
    size_t len = args[1]->as.string.length;
    size_t escritos = fwrite(args[1]->as.string.data, 1, len, f);
    fclose(f);
    return gc_alloc_bool(vm, escritos == len);
}

void leviata_resources_register(LvVM *vm) {
    if (!vm) return;

    /* Construtores públicos. */
    vm_register_native(vm, "imagem", resource_imagem, 1);
    vm_register_native(vm, "som", resource_som, 1);
    vm_register_native(vm, "arquivo", resource_arquivo, 1);

    /* tipo() passa a reconhecer os recursos concretos. */
    vm_register_native(vm, "tipo", resource_tipo, 1);

    /* Campos ocultos. O '@' não é um identificador válido da linguagem. */
    vm_register_native(vm, "@campo.caminho", field_caminho, 1);
    vm_register_native(vm, "@campo.largura", field_largura, 1);
    vm_register_native(vm, "@campo.altura", field_altura, 1);
    vm_register_native(vm, "@campo.volume", field_volume, 1);

    /* Métodos ocultos; o compilador injeta o receptor como argumento 0. */
    vm_register_native(vm, "@metodo.desenhe", method_desenhe, -1);
    vm_register_native(vm, "@metodo.toque", method_toque, 1);
    vm_register_native(vm, "@metodo.volume", method_volume, -1);
    vm_register_native(vm, "@metodo.leia", method_leia, 1);
    vm_register_native(vm, "@metodo.escreva", method_escreva, 2);
    vm_register_native(vm, "@metodo.adicione", method_adicione, 2);
}