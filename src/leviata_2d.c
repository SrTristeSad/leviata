/* ============================================================================
 * MOTOR LEVIATA - MODULO 2D AVANCADO
 *
 * Recursos de alto nivel construidos sobre o backend FFI base:
 *   - camera 2D
 *   - colisao AABB, ponto/retangulo e circulos
 *   - spritesheet e animacao horizontal
 *   - tilemap baseado em listas PT
 *   - gerenciamento simples de cenas por nome
 *
 * O modulo continua compilando sem Raylib. Nesse caso os recursos de desenho
 * viram no-op, enquanto colisao e cenas continuam totalmente funcionais.
 * ============================================================================ */
#include "leviata_2d.h"
#include "leviata_ffi.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef LVA_USE_RAYLIB
#include <raylib.h>
#endif

#define LVA_2D_TEXTURE_CACHE_MAX 128
#define LVA_SCENE_NAME_MAX       128

static LvTexture *g_2d_textures[LVA_2D_TEXTURE_CACHE_MAX];
static int g_2d_texture_count = 0;

static LvCameraState g_camera = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };
static int g_camera_active = 0;

static char g_scene_current[LVA_SCENE_NAME_MAX] = "";
static char g_scene_previous[LVA_SCENE_NAME_MAX] = "";
static int64_t g_scene_version = 0;

/* ============================================================
 * Helpers
 * ============================================================ */
static int arg_is_number(LvObject *o) {
    return o && (o->type == LV_TYPE_INT || o->type == LV_TYPE_FLOAT);
}

static double arg_number(LvObject *o) {
    if (!o) return 0.0;
    if (o->type == LV_TYPE_FLOAT) return o->as.f_val;
    if (o->type == LV_TYPE_INT) return (double)o->as.i_val;
    return 0.0;
}

static int64_t arg_integer(LvObject *o) {
    if (!o) return 0;
    if (o->type == LV_TYPE_INT || o->type == LV_TYPE_BOOL) return o->as.i_val;
    if (o->type == LV_TYPE_FLOAT) return (int64_t)o->as.f_val;
    return 0;
}

static const char *arg_string(LvObject *o) {
    return (o && o->type == LV_TYPE_STRING) ? o->as.string.data : NULL;
}

static int arg_truthy(LvObject *o) {
    if (!o || o->type == LV_TYPE_NULL) return 0;
    if (o->type == LV_TYPE_BOOL || o->type == LV_TYPE_INT) return o->as.i_val != 0;
    if (o->type == LV_TYPE_FLOAT) return o->as.f_val != 0.0;
    return 1;
}

static double engine_time_seconds(void) {
#ifdef LVA_USE_RAYLIB
    return GetTime();
#else
    return (double)clock() / (double)CLOCKS_PER_SEC;
#endif
}

static LvTexture *texture_cache_get(const char *path) {
    if (!path || !path[0]) return NULL;

    for (int i = 0; i < g_2d_texture_count; i++) {
        if (g_2d_textures[i] && strcmp(g_2d_textures[i]->path, path) == 0)
            return g_2d_textures[i];
    }

    if (g_2d_texture_count >= LVA_2D_TEXTURE_CACHE_MAX) {
        fprintf(stderr, "[LEVIATA][2D] Cache de texturas cheio (%d).\n",
                LVA_2D_TEXTURE_CACHE_MAX);
        return NULL;
    }

    LvTexture *tex = game_load_texture(path);
    if (!tex) {
        fprintf(stderr, "[LEVIATA][2D] Nao foi possivel carregar textura: %s\n", path);
        return NULL;
    }

    g_2d_textures[g_2d_texture_count++] = tex;
    return tex;
}

static void texture_cache_clear(void) {
    for (int i = 0; i < g_2d_texture_count; i++) {
        if (g_2d_textures[i]) game_unload_texture(g_2d_textures[i]);
        g_2d_textures[i] = NULL;
    }
    g_2d_texture_count = 0;
}

static int list_tile_value(LvObject *list, int index, int *out_value) {
    if (!list || list->type != LV_TYPE_LIST || index < 0 || index >= list->as.list.count)
        return 0;

    LvObject *item = list->as.list.items[index];
    if (!item || (item->type != LV_TYPE_INT && item->type != LV_TYPE_FLOAT))
        return 0;

    *out_value = (int)arg_integer(item);
    return 1;
}

/* ============================================================
 * Colisao
 * ============================================================ */
int leviata_2d_rects_collide(float ax, float ay, float aw, float ah,
                             float bx, float by, float bw, float bh) {
    if (aw <= 0.0f || ah <= 0.0f || bw <= 0.0f || bh <= 0.0f) return 0;
    return ax < bx + bw && ax + aw > bx && ay < by + bh && ay + ah > by;
}

int leviata_2d_point_in_rect(float px, float py,
                             float rx, float ry, float rw, float rh) {
    if (rw <= 0.0f || rh <= 0.0f) return 0;
    return px >= rx && px <= rx + rw && py >= ry && py <= ry + rh;
}

int leviata_2d_circles_collide(float ax, float ay, float ar,
                               float bx, float by, float br) {
    if (ar < 0.0f || br < 0.0f) return 0;
    float dx = ax - bx;
    float dy = ay - by;
    float rr = ar + br;
    return dx * dx + dy * dy <= rr * rr;
}

/* ============================================================
 * Camera
 * ============================================================ */
static void camera_begin(void) {
    if (g_camera_active) return;
#ifdef LVA_USE_RAYLIB
    Camera2D cam = {0};
    cam.target = (Vector2){ g_camera.target_x, g_camera.target_y };
    cam.offset = (Vector2){ g_camera.offset_x, g_camera.offset_y };
    cam.rotation = g_camera.rotation;
    cam.zoom = g_camera.zoom <= 0.0f ? 1.0f : g_camera.zoom;
    BeginMode2D(cam);
#endif
    g_camera_active = 1;
}

static void camera_end(void) {
    if (!g_camera_active) return;
#ifdef LVA_USE_RAYLIB
    EndMode2D();
#endif
    g_camera_active = 0;
}

/* ============================================================
 * Bindings de camera
 * ============================================================ */
static LvObject *native_camera_configure(LvVM *vm, int argc, LvObject **args) {
    if (argc < 4) {
        fprintf(stderr, "[LEVIATA][2D] jogo_camera_configurar(alvo_x, alvo_y, zoom, rotacao, offset_x?, offset_y?)\n");
        return vm->nil_singleton;
    }

    g_camera.target_x = (float)arg_number(args[0]);
    g_camera.target_y = (float)arg_number(args[1]);
    g_camera.zoom = (float)arg_number(args[2]);
    if (g_camera.zoom <= 0.0f) g_camera.zoom = 1.0f;
    g_camera.rotation = (float)arg_number(args[3]);

    if (argc >= 6) {
        g_camera.offset_x = (float)arg_number(args[4]);
        g_camera.offset_y = (float)arg_number(args[5]);
    } else {
#ifdef LVA_USE_RAYLIB
        g_camera.offset_x = IsWindowReady() ? (float)GetScreenWidth() * 0.5f : 0.0f;
        g_camera.offset_y = IsWindowReady() ? (float)GetScreenHeight() * 0.5f : 0.0f;
#else
        g_camera.offset_x = 0.0f;
        g_camera.offset_y = 0.0f;
#endif
    }

    return vm->nil_singleton;
}

static LvObject *native_camera_follow(LvVM *vm, int argc, LvObject **args) {
    if (argc >= 2) {
        g_camera.target_x = (float)arg_number(args[0]);
        g_camera.target_y = (float)arg_number(args[1]);
    }
    return vm->nil_singleton;
}

static LvObject *native_camera_begin(LvVM *vm, int argc, LvObject **args) {
    (void)argc; (void)args;
    camera_begin();
    return vm->nil_singleton;
}

static LvObject *native_camera_end(LvVM *vm, int argc, LvObject **args) {
    (void)argc; (void)args;
    camera_end();
    return vm->nil_singleton;
}

/* ============================================================
 * Bindings de colisao
 * ============================================================ */
static LvObject *native_collision_rects(LvVM *vm, int argc, LvObject **args) {
    if (argc < 8) return gc_alloc_bool(vm, 0);
    return gc_alloc_bool(vm, leviata_2d_rects_collide(
        (float)arg_number(args[0]), (float)arg_number(args[1]),
        (float)arg_number(args[2]), (float)arg_number(args[3]),
        (float)arg_number(args[4]), (float)arg_number(args[5]),
        (float)arg_number(args[6]), (float)arg_number(args[7])));
}

static LvObject *native_collision_point_rect(LvVM *vm, int argc, LvObject **args) {
    if (argc < 6) return gc_alloc_bool(vm, 0);
    return gc_alloc_bool(vm, leviata_2d_point_in_rect(
        (float)arg_number(args[0]), (float)arg_number(args[1]),
        (float)arg_number(args[2]), (float)arg_number(args[3]),
        (float)arg_number(args[4]), (float)arg_number(args[5])));
}

static LvObject *native_collision_circles(LvVM *vm, int argc, LvObject **args) {
    if (argc < 6) return gc_alloc_bool(vm, 0);
    return gc_alloc_bool(vm, leviata_2d_circles_collide(
        (float)arg_number(args[0]), (float)arg_number(args[1]), (float)arg_number(args[2]),
        (float)arg_number(args[3]), (float)arg_number(args[4]), (float)arg_number(args[5])));
}

/* ============================================================
 * Spritesheet / animacao
 * ============================================================ */
static LvObject *native_sprite_region(LvVM *vm, int argc, LvObject **args) {
    if (argc < 9 || !arg_string(args[0])) return vm->nil_singleton;

    LvTexture *tex = texture_cache_get(arg_string(args[0]));
    if (!tex) return vm->nil_singleton;

    float sx = (float)arg_number(args[1]);
    float sy = (float)arg_number(args[2]);
    float sw = (float)arg_number(args[3]);
    float sh = (float)arg_number(args[4]);
    float dx = (float)arg_number(args[5]);
    float dy = (float)arg_number(args[6]);
    float dw = (float)arg_number(args[7]);
    float dh = (float)arg_number(args[8]);
    int flip_x = argc >= 10 ? arg_truthy(args[9]) : 0;
    int flip_y = argc >= 11 ? arg_truthy(args[10]) : 0;

    if (flip_x) sw = -fabsf(sw);
    if (flip_y) sh = -fabsf(sh);

    game_draw_texture(tex,
                      (LvRect){ sx, sy, sw, sh },
                      (LvRect){ dx, dy, dw, dh },
                      (LvColor){ 255, 255, 255, 255 });
    return vm->nil_singleton;
}

static int animation_frame(int frames, double fps) {
    if (frames <= 1 || fps <= 0.0) return 0;
    int64_t raw = (int64_t)floor(engine_time_seconds() * fps);
    int frame = (int)(raw % frames);
    return frame < 0 ? frame + frames : frame;
}

static LvObject *native_animation_frame(LvVM *vm, int argc, LvObject **args) {
    if (argc < 2) return gc_alloc_int(vm, 0);
    return gc_alloc_int(vm, animation_frame((int)arg_integer(args[0]), arg_number(args[1])));
}

static LvObject *native_draw_animation(LvVM *vm, int argc, LvObject **args) {
    if (argc < 7 || !arg_string(args[0])) return vm->nil_singleton;

    LvTexture *tex = texture_cache_get(arg_string(args[0]));
    if (!tex) return vm->nil_singleton;

    float x = (float)arg_number(args[1]);
    float y = (float)arg_number(args[2]);
    float frame_w = (float)arg_number(args[3]);
    float frame_h = (float)arg_number(args[4]);
    int frames = (int)arg_integer(args[5]);
    double fps = arg_number(args[6]);
    int row = argc >= 8 ? (int)arg_integer(args[7]) : 0;
    float scale = argc >= 9 ? (float)arg_number(args[8]) : 1.0f;
    int flip_x = argc >= 10 ? arg_truthy(args[9]) : 0;

    if (frame_w <= 0.0f || frame_h <= 0.0f || frames <= 0) return vm->nil_singleton;
    if (scale <= 0.0f) scale = 1.0f;

    int frame = animation_frame(frames, fps);
    float src_w = flip_x ? -frame_w : frame_w;
    LvRect src = { frame * frame_w, row * frame_h, src_w, frame_h };
    LvRect dst = { x, y, frame_w * scale, frame_h * scale };
    game_draw_texture(tex, src, dst, (LvColor){ 255, 255, 255, 255 });
    return gc_alloc_int(vm, frame);
}

/* ============================================================
 * Tilemap
 * ============================================================ */
static LvObject *native_draw_tilemap(LvVM *vm, int argc, LvObject **args) {
    if (argc < 7 || !arg_string(args[0]) || !args[1] || args[1]->type != LV_TYPE_LIST)
        return vm->nil_singleton;

    LvTexture *tex = texture_cache_get(arg_string(args[0]));
    if (!tex) return vm->nil_singleton;

    LvObject *map = args[1];
    int columns = (int)arg_integer(args[2]);
    float tile_w = (float)arg_number(args[3]);
    float tile_h = (float)arg_number(args[4]);
    float origin_x = (float)arg_number(args[5]);
    float origin_y = (float)arg_number(args[6]);
    float scale = argc >= 8 ? (float)arg_number(args[7]) : 1.0f;
    int tileset_columns = argc >= 9 ? (int)arg_integer(args[8]) : 0;

    if (columns <= 0 || tile_w <= 0.0f || tile_h <= 0.0f) return vm->nil_singleton;
    if (scale <= 0.0f) scale = 1.0f;
    if (tileset_columns <= 0) tileset_columns = (int)((float)tex->width / tile_w);
    if (tileset_columns <= 0) tileset_columns = 1;

    for (int i = 0; i < map->as.list.count; i++) {
        int tile = -1;
        if (!list_tile_value(map, i, &tile) || tile < 0) continue;

        int map_x = i % columns;
        int map_y = i / columns;
        int src_x = tile % tileset_columns;
        int src_y = tile / tileset_columns;

        LvRect src = { src_x * tile_w, src_y * tile_h, tile_w, tile_h };
        LvRect dst = { origin_x + map_x * tile_w * scale,
                       origin_y + map_y * tile_h * scale,
                       tile_w * scale, tile_h * scale };
        game_draw_texture(tex, src, dst, (LvColor){ 255, 255, 255, 255 });
    }

    return vm->nil_singleton;
}

static LvObject *native_collision_tilemap(LvVM *vm, int argc, LvObject **args) {
    if (argc < 7 || !args[4] || args[4]->type != LV_TYPE_LIST)
        return gc_alloc_bool(vm, 0);

    float x = (float)arg_number(args[0]);
    float y = (float)arg_number(args[1]);
    float w = (float)arg_number(args[2]);
    float h = (float)arg_number(args[3]);
    LvObject *map = args[4];
    int columns = (int)arg_integer(args[5]);
    float tile_size = (float)arg_number(args[6]);
    float origin_x = argc >= 8 ? (float)arg_number(args[7]) : 0.0f;
    float origin_y = argc >= 9 ? (float)arg_number(args[8]) : 0.0f;

    if (columns <= 0 || tile_size <= 0.0f || w <= 0.0f || h <= 0.0f)
        return gc_alloc_bool(vm, 0);

    int rows = (map->as.list.count + columns - 1) / columns;
    int min_x = (int)floorf((x - origin_x) / tile_size);
    int min_y = (int)floorf((y - origin_y) / tile_size);
    int max_x = (int)floorf((x + w - 0.001f - origin_x) / tile_size);
    int max_y = (int)floorf((y + h - 0.001f - origin_y) / tile_size);

    for (int ty = min_y; ty <= max_y; ty++) {
        for (int tx = min_x; tx <= max_x; tx++) {
            if (tx < 0 || ty < 0 || tx >= columns || ty >= rows) continue;
            int index = ty * columns + tx;
            int tile = -1;
            if (list_tile_value(map, index, &tile) && tile >= 0)
                return gc_alloc_bool(vm, 1);
        }
    }

    return gc_alloc_bool(vm, 0);
}

/* ============================================================
 * Cenas
 * ============================================================ */
static void scene_set(const char *name) {
    if (!name) name = "";
    if (strcmp(g_scene_current, name) == 0) return;

    strncpy(g_scene_previous, g_scene_current, sizeof(g_scene_previous) - 1);
    g_scene_previous[sizeof(g_scene_previous) - 1] = '\0';
    strncpy(g_scene_current, name, sizeof(g_scene_current) - 1);
    g_scene_current[sizeof(g_scene_current) - 1] = '\0';
    g_scene_version++;
}

static LvObject *native_scene_change(LvVM *vm, int argc, LvObject **args) {
    if (argc >= 1 && arg_string(args[0])) scene_set(arg_string(args[0]));
    return vm->nil_singleton;
}

static LvObject *native_scene_current(LvVM *vm, int argc, LvObject **args) {
    (void)argc; (void)args;
    return gc_alloc_string(vm, g_scene_current, strlen(g_scene_current));
}

static LvObject *native_scene_previous(LvVM *vm, int argc, LvObject **args) {
    (void)argc; (void)args;
    return gc_alloc_string(vm, g_scene_previous, strlen(g_scene_previous));
}

static LvObject *native_scene_is(LvVM *vm, int argc, LvObject **args) {
    if (argc < 1 || !arg_string(args[0])) return gc_alloc_bool(vm, 0);
    return gc_alloc_bool(vm, strcmp(g_scene_current, arg_string(args[0])) == 0);
}

static LvObject *native_scene_version(LvVM *vm, int argc, LvObject **args) {
    (void)argc; (void)args;
    return gc_alloc_int(vm, g_scene_version);
}

/* Sobrescreve jogo_fechar para garantir que o cache deste modulo seja limpo. */
static LvObject *native_2d_game_close(LvVM *vm, int argc, LvObject **args) {
    (void)argc; (void)args;
    camera_end();
    texture_cache_clear();
    game_close();
    return vm->nil_singleton;
}

/* ============================================================
 * Registro / shutdown
 * ============================================================ */
void leviata_2d_register_runtime(LvVM *vm) {
    if (!vm) return;

    vm_register_native(vm, "jogo_camera_configurar",       native_camera_configure,      -1);
    vm_register_native(vm, "jogo_camera_seguir",           native_camera_follow,          2);
    vm_register_native(vm, "jogo_camera_iniciar",          native_camera_begin,           0);
    vm_register_native(vm, "jogo_camera_finalizar",        native_camera_end,             0);

    vm_register_native(vm, "jogo_colisao_retangulos",      native_collision_rects,        8);
    vm_register_native(vm, "jogo_ponto_no_retangulo",      native_collision_point_rect,   6);
    vm_register_native(vm, "jogo_colisao_circulos",        native_collision_circles,      6);

    vm_register_native(vm, "jogo_desenhar_sprite_regiao",  native_sprite_region,         -1);
    vm_register_native(vm, "jogo_animacao_frame",          native_animation_frame,        2);
    vm_register_native(vm, "jogo_desenhar_animacao",       native_draw_animation,        -1);

    vm_register_native(vm, "jogo_desenhar_tilemap",        native_draw_tilemap,          -1);
    vm_register_native(vm, "jogo_colisao_tilemap",         native_collision_tilemap,     -1);

    vm_register_native(vm, "cena_mudar",                   native_scene_change,           1);
    vm_register_native(vm, "cena_atual",                   native_scene_current,          0);
    vm_register_native(vm, "cena_anterior",                native_scene_previous,         0);
    vm_register_native(vm, "cena_e",                       native_scene_is,               1);
    vm_register_native(vm, "cena_versao",                  native_scene_version,          0);

    vm_register_native(vm, "jogo_fechar",                  native_2d_game_close,          0);
}

void leviata_2d_shutdown(void) {
    g_camera_active = 0;
    texture_cache_clear();
    g_scene_current[0] = '\0';
    g_scene_previous[0] = '\0';
    g_scene_version = 0;
}
