/* ============================================================================
 * MOTOR LEVIATA - WORLD / ENTIDADES / FISICA / GAMEPLAY
 * ============================================================================ */
#include "leviata_world.h"
#include "leviata_ffi.h"
#include "leviata_2d.h"

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef LVA_USE_RAYLIB
#include <raylib.h>
#endif

#define COMP_TRANSFORM 1
#define COMP_PHYSICS   2
#define COMP_COLLIDER  4
#define COMP_SPRITE    8
#define COMP_TRIGGER   16

static LvEntity g_entities[LVA_WORLD_ENTITY_MAX];
static int g_next_entity_id = 1;

static int g_last_map_width = 0;
static int g_last_map_height = 0;

static struct {
    float target_x, target_y;
    float offset_x, offset_y;
    float rotation, zoom;
    float min_x, min_y, max_x, max_y;
    float smoothing;
    int bounds_enabled;
    int active;
} g_camera = {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0};

/* ============================================================
 * Helpers
 * ============================================================ */
static double arg_number(LvObject *o) {
    if (!o) return 0.0;
    if (o->type == LV_TYPE_FLOAT) return o->as.f_val;
    if (o->type == LV_TYPE_INT || o->type == LV_TYPE_BOOL) return (double)o->as.i_val;
    return 0.0;
}

static int64_t arg_integer(LvObject *o) {
    if (!o) return 0;
    if (o->type == LV_TYPE_FLOAT) return (int64_t)o->as.f_val;
    if (o->type == LV_TYPE_INT || o->type == LV_TYPE_BOOL) return o->as.i_val;
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

static int clamp_byte(int value) {
    if (value < 0) return 0;
    if (value > 255) return 255;
    return value;
}

static LvEntity *entity_find(int id) {
    if (id <= 0) return NULL;
    for (int i = 0; i < LVA_WORLD_ENTITY_MAX; i++) {
        if (g_entities[i].active && g_entities[i].id == id)
            return &g_entities[i];
    }
    return NULL;
}

static void entity_release_sprite(LvEntity *e) {
    if (!e) return;
    if (e->texture) {
        game_unload_texture(e->texture);
        e->texture = NULL;
    }
    e->sprite_enabled = 0;
    e->sprite_path[0] = '\0';
    e->sprite_time = 0.0f;
}

static void entity_release_all_sprites(void) {
    for (int i = 0; i < LVA_WORLD_ENTITY_MAX; i++) {
        if (g_entities[i].active)
            entity_release_sprite(&g_entities[i]);
    }
}

static LvEntity *entity_alloc(void) {
    for (int i = 0; i < LVA_WORLD_ENTITY_MAX; i++) {
        if (!g_entities[i].active) {
            memset(&g_entities[i], 0, sizeof(g_entities[i]));
            g_entities[i].active = 1;
            g_entities[i].id = g_next_entity_id++;
            g_entities[i].width = 32.0f;
            g_entities[i].height = 32.0f;
            g_entities[i].collider_enabled = 1;
            g_entities[i].sprite_scale = 1.0f;
            g_entities[i].sprite_frames = 1;
            g_entities[i].sprite_playing = 1;
            return &g_entities[i];
        }
    }
    return NULL;
}

static int entity_overlaps(const LvEntity *a, const LvEntity *b) {
    if (!a || !b || !a->active || !b->active) return 0;
    if (a->width <= 0.0f || a->height <= 0.0f ||
        b->width <= 0.0f || b->height <= 0.0f) return 0;
    return a->x < b->x + b->width &&
           a->x + a->width > b->x &&
           a->y < b->y + b->height &&
           a->y + a->height > b->y;
}

static float entity_center_x(const LvEntity *e) {
    return e ? e->x + e->width * 0.5f : 0.0f;
}

static float entity_center_y(const LvEntity *e) {
    return e ? e->y + e->height * 0.5f : 0.0f;
}

static void entity_advance_animation(LvEntity *e, float dt) {
    if (!e || !e->sprite_enabled || !e->sprite_playing || e->sprite_fps <= 0.0f)
        return;
    if (dt < 0.0f) dt = 0.0f;
    if (dt > 0.25f) dt = 0.25f;
    e->sprite_time += dt;
}

static int entity_animation_frame(const LvEntity *e) {
    if (!e || e->sprite_frames <= 1 || e->sprite_fps <= 0.0f) return 0;
    int frame = (int)floorf(e->sprite_time * e->sprite_fps);
    frame %= e->sprite_frames;
    return frame < 0 ? frame + e->sprite_frames : frame;
}

static int list_push(LvObject *list, LvObject *value) {
    if (!list || list->type != LV_TYPE_LIST) return 0;
    if (list->as.list.count >= list->as.list.capacity) {
        int next = list->as.list.capacity > 0 ? list->as.list.capacity * 2 : 8;
        LvObject **items = (LvObject**)realloc(
            list->as.list.items, next * sizeof(LvObject*));
        if (!items) return 0;
        list->as.list.items = items;
        list->as.list.capacity = next;
    }
    list->as.list.items[list->as.list.count++] = value;
    return 1;
}

static int tile_value(LvObject *map, int index, int *out) {
    if (!map || map->type != LV_TYPE_LIST ||
        index < 0 || index >= map->as.list.count) return 0;
    LvObject *v = map->as.list.items[index];
    if (!v || (v->type != LV_TYPE_INT && v->type != LV_TYPE_FLOAT)) return 0;
    *out = (int)arg_integer(v);
    return 1;
}

static int collides_map(float x, float y, float w, float h,
                        LvObject *map, int columns,
                        float tile_w, float tile_h,
                        float origin_x, float origin_y) {
    if (!map || map->type != LV_TYPE_LIST || columns <= 0 ||
        tile_w <= 0 || tile_h <= 0) return 0;

    int rows = (map->as.list.count + columns - 1) / columns;
    int min_x = (int)floorf((x - origin_x) / tile_w);
    int min_y = (int)floorf((y - origin_y) / tile_h);
    int max_x = (int)floorf((x + w - 0.001f - origin_x) / tile_w);
    int max_y = (int)floorf((y + h - 0.001f - origin_y) / tile_h);

    for (int ty = min_y; ty <= max_y; ty++) {
        for (int tx = min_x; tx <= max_x; tx++) {
            if (tx < 0 || ty < 0 || tx >= columns || ty >= rows) continue;
            int value = -1;
            if (tile_value(map, ty * columns + tx, &value) && value >= 0)
                return 1;
        }
    }
    return 0;
}

static int move_axis(LvEntity *e, float delta, int x_axis,
                     LvObject *map, int columns,
                     float tile_w, float tile_h,
                     float origin_x, float origin_y) {
    if (!e || delta == 0.0f) return 0;
    int steps = (int)ceilf(fabsf(delta));
    if (steps < 1) steps = 1;
    float step = delta / (float)steps;
    int hit = 0;

    for (int i = 0; i < steps; i++) {
        float nx = e->x + (x_axis ? step : 0.0f);
        float ny = e->y + (x_axis ? 0.0f : step);
        if (e->collider_enabled &&
            collides_map(nx, ny, e->width, e->height,
                         map, columns, tile_w, tile_h, origin_x, origin_y)) {
            hit = 1;
            break;
        }
        e->x = nx;
        e->y = ny;
    }
    return hit;
}

static void apply_physics(LvEntity *e, float dt) {
    if (!e || !e->physics_enabled) return;
    if (dt < 0.0f) dt = 0.0f;
    if (dt > 0.25f) dt = 0.25f;
    e->vy += e->gravity * dt;
    float factor = 1.0f - e->drag * dt;
    if (factor < 0.0f) factor = 0.0f;
    e->vx *= factor;
}

/* ============================================================
 * Camera
 * ============================================================ */
static void camera_clamp(void) {
    if (!g_camera.bounds_enabled) return;
    float min_tx = g_camera.min_x;
    float max_tx = g_camera.max_x;
    float min_ty = g_camera.min_y;
    float max_ty = g_camera.max_y;

#ifdef LVA_USE_RAYLIB
    float zoom = g_camera.zoom > 0.0f ? g_camera.zoom : 1.0f;
    float half_w = IsWindowReady() ? (float)GetScreenWidth() * 0.5f / zoom : 0.0f;
    float half_h = IsWindowReady() ? (float)GetScreenHeight() * 0.5f / zoom : 0.0f;
    min_tx += half_w;
    max_tx -= half_w;
    min_ty += half_h;
    max_ty -= half_h;
#endif

    if (min_tx > max_tx) {
        g_camera.target_x = (g_camera.min_x + g_camera.max_x) * 0.5f;
    } else {
        if (g_camera.target_x < min_tx) g_camera.target_x = min_tx;
        if (g_camera.target_x > max_tx) g_camera.target_x = max_tx;
    }

    if (min_ty > max_ty) {
        g_camera.target_y = (g_camera.min_y + g_camera.max_y) * 0.5f;
    } else {
        if (g_camera.target_y < min_ty) g_camera.target_y = min_ty;
        if (g_camera.target_y > max_ty) g_camera.target_y = max_ty;
    }
}

static void camera_follow_point(float x, float y) {
    if (g_camera.smoothing <= 0.0f) {
        g_camera.target_x = x;
        g_camera.target_y = y;
    } else {
        float dt = game_get_delta_time();
        if (dt < 0.0f) dt = 0.0f;
        if (dt > 0.25f) dt = 0.25f;
        float alpha = 1.0f - expf(-g_camera.smoothing * dt);
        if (alpha < 0.0f) alpha = 0.0f;
        if (alpha > 1.0f) alpha = 1.0f;
        g_camera.target_x += (x - g_camera.target_x) * alpha;
        g_camera.target_y += (y - g_camera.target_y) * alpha;
    }
    camera_clamp();
}

static void camera_begin(void) {
    if (g_camera.active) return;
    camera_clamp();
#ifdef LVA_USE_RAYLIB
    Camera2D cam = {0};
    cam.target = (Vector2){g_camera.target_x, g_camera.target_y};
    cam.offset = (Vector2){g_camera.offset_x, g_camera.offset_y};
    cam.rotation = g_camera.rotation;
    cam.zoom = g_camera.zoom > 0.0f ? g_camera.zoom : 1.0f;
    BeginMode2D(cam);
#endif
    g_camera.active = 1;
}

static void camera_end(void) {
    if (!g_camera.active) return;
#ifdef LVA_USE_RAYLIB
    EndMode2D();
#endif
    g_camera.active = 0;
}

static LvObject *native_camera_configure(LvVM *vm, int argc, LvObject **args) {
    if (argc < 4) return vm->nil_singleton;
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

    camera_clamp();
    return vm->nil_singleton;
}

static LvObject *native_camera_follow(LvVM *vm, int argc, LvObject **args) {
    if (argc >= 2)
        camera_follow_point((float)arg_number(args[0]), (float)arg_number(args[1]));
    return vm->nil_singleton;
}

static LvObject *native_camera_smoothing(LvVM *vm, int argc, LvObject **args) {
    if (argc < 1) return gc_alloc_float(vm, g_camera.smoothing);
    g_camera.smoothing = (float)arg_number(args[0]);
    if (g_camera.smoothing < 0.0f) g_camera.smoothing = 0.0f;
    return gc_alloc_float(vm, g_camera.smoothing);
}

static LvObject *native_camera_bounds(LvVM *vm, int argc, LvObject **args) {
    if (argc < 4) return gc_alloc_bool(vm, 0);
    g_camera.min_x = (float)arg_number(args[0]);
    g_camera.min_y = (float)arg_number(args[1]);
    g_camera.max_x = (float)arg_number(args[2]);
    g_camera.max_y = (float)arg_number(args[3]);
    g_camera.bounds_enabled =
        g_camera.max_x > g_camera.min_x && g_camera.max_y > g_camera.min_y;
    camera_clamp();
    return gc_alloc_bool(vm, g_camera.bounds_enabled);
}

static LvObject *native_camera_bounds_clear(LvVM *vm, int argc, LvObject **args) {
    (void)argc;
    (void)args;
    g_camera.bounds_enabled = 0;
    return vm->nil_singleton;
}

static LvObject *native_camera_begin(LvVM *vm, int argc, LvObject **args) {
    (void)argc;
    (void)args;
    camera_begin();
    return vm->nil_singleton;
}

static LvObject *native_camera_end(LvVM *vm, int argc, LvObject **args) {
    (void)argc;
    (void)args;
    camera_end();
    return vm->nil_singleton;
}

/* ============================================================
 * Entidades / componentes
 * ============================================================ */
static LvObject *native_entity_create(LvVM *vm, int argc, LvObject **args) {
    if (argc < 5 || !arg_string(args[0])) return gc_alloc_int(vm, 0);
    LvEntity *e = entity_alloc();
    if (!e) return gc_alloc_int(vm, 0);

    strncpy(e->name, arg_string(args[0]), sizeof(e->name) - 1);
    e->name[sizeof(e->name) - 1] = '\0';
    e->x = (float)arg_number(args[1]);
    e->y = (float)arg_number(args[2]);
    e->width = (float)arg_number(args[3]);
    e->height = (float)arg_number(args[4]);
    if (e->width <= 0.0f) e->width = 1.0f;
    if (e->height <= 0.0f) e->height = 1.0f;
    return gc_alloc_int(vm, e->id);
}

static LvObject *native_entity_destroy(LvVM *vm, int argc, LvObject **args) {
    LvEntity *e = argc >= 1 ? entity_find((int)arg_integer(args[0])) : NULL;
    if (!e) return gc_alloc_bool(vm, 0);
    entity_release_sprite(e);
    memset(e, 0, sizeof(*e));
    return gc_alloc_bool(vm, 1);
}

static LvObject *native_entity_find_by_name(LvVM *vm, int argc, LvObject **args) {
    const char *name = argc >= 1 ? arg_string(args[0]) : NULL;
    if (!name) return gc_alloc_int(vm, 0);
    for (int i = 0; i < LVA_WORLD_ENTITY_MAX; i++) {
        if (g_entities[i].active && strcmp(g_entities[i].name, name) == 0)
            return gc_alloc_int(vm, g_entities[i].id);
    }
    return gc_alloc_int(vm, 0);
}

static LvObject *native_entity_position(LvVM *vm, int argc, LvObject **args) {
    LvEntity *e = argc >= 3 ? entity_find((int)arg_integer(args[0])) : NULL;
    if (!e) return gc_alloc_bool(vm, 0);
    e->x = (float)arg_number(args[1]);
    e->y = (float)arg_number(args[2]);
    return gc_alloc_bool(vm, 1);
}

static LvObject *native_entity_size(LvVM *vm, int argc, LvObject **args) {
    LvEntity *e = argc >= 3 ? entity_find((int)arg_integer(args[0])) : NULL;
    if (!e) return gc_alloc_bool(vm, 0);
    float w = (float)arg_number(args[1]);
    float h = (float)arg_number(args[2]);
    if (w <= 0.0f || h <= 0.0f) return gc_alloc_bool(vm, 0);
    e->width = w;
    e->height = h;
    return gc_alloc_bool(vm, 1);
}

static LvObject *native_entity_velocity(LvVM *vm, int argc, LvObject **args) {
    LvEntity *e = argc >= 3 ? entity_find((int)arg_integer(args[0])) : NULL;
    if (!e) return gc_alloc_bool(vm, 0);
    e->vx = (float)arg_number(args[1]);
    e->vy = (float)arg_number(args[2]);
    return gc_alloc_bool(vm, 1);
}

static LvObject *native_entity_physics(LvVM *vm, int argc, LvObject **args) {
    LvEntity *e = argc >= 2 ? entity_find((int)arg_integer(args[0])) : NULL;
    if (!e) return gc_alloc_bool(vm, 0);
    e->physics_enabled = 1;
    e->gravity = (float)arg_number(args[1]);
    e->drag = argc >= 3 ? (float)arg_number(args[2]) : 0.0f;
    if (e->drag < 0.0f) e->drag = 0.0f;
    return gc_alloc_bool(vm, 1);
}

static LvObject *native_entity_collider(LvVM *vm, int argc, LvObject **args) {
    LvEntity *e = argc >= 2 ? entity_find((int)arg_integer(args[0])) : NULL;
    if (!e) return gc_alloc_bool(vm, 0);
    e->collider_enabled = arg_truthy(args[1]);
    return gc_alloc_bool(vm, 1);
}

static LvObject *native_entity_trigger(LvVM *vm, int argc, LvObject **args) {
    LvEntity *e = argc >= 2 ? entity_find((int)arg_integer(args[0])) : NULL;
    if (!e) return gc_alloc_bool(vm, 0);
    e->trigger_enabled = arg_truthy(args[1]);
    if (e->trigger_enabled) e->collider_enabled = 1;
    return gc_alloc_bool(vm, 1);
}

static LvObject *native_entity_collision(LvVM *vm, int argc, LvObject **args) {
    if (argc < 2) return gc_alloc_bool(vm, 0);
    LvEntity *a = entity_find((int)arg_integer(args[0]));
    LvEntity *b = entity_find((int)arg_integer(args[1]));
    if (!a || !b || !a->collider_enabled || !b->collider_enabled)
        return gc_alloc_bool(vm, 0);
    return gc_alloc_bool(vm, entity_overlaps(a, b));
}

static LvObject *native_entity_distance(LvVM *vm, int argc, LvObject **args) {
    if (argc < 2) return gc_alloc_float(vm, 0.0);
    LvEntity *a = entity_find((int)arg_integer(args[0]));
    LvEntity *b = entity_find((int)arg_integer(args[1]));
    if (!a || !b) return gc_alloc_float(vm, 0.0);
    float dx = entity_center_x(a) - entity_center_x(b);
    float dy = entity_center_y(a) - entity_center_y(b);
    return gc_alloc_float(vm, sqrtf(dx * dx + dy * dy));
}

static LvObject *native_entity_update(LvVM *vm, int argc, LvObject **args) {
    LvEntity *e = argc >= 2 ? entity_find((int)arg_integer(args[0])) : NULL;
    if (!e) return gc_alloc_bool(vm, 0);
    float dt = (float)arg_number(args[1]);
    apply_physics(e, dt);
    e->x += e->vx * dt;
    e->y += e->vy * dt;
    e->grounded = 0;
    entity_advance_animation(e, dt);
    return gc_alloc_bool(vm, 1);
}

static LvObject *native_entity_update_tilemap(LvVM *vm, int argc, LvObject **args) {
    if (argc < 6 || !args[2] || args[2]->type != LV_TYPE_LIST)
        return gc_alloc_bool(vm, 0);

    LvEntity *e = entity_find((int)arg_integer(args[0]));
    if (!e) return gc_alloc_bool(vm, 0);

    float dt = (float)arg_number(args[1]);
    LvObject *map = args[2];
    int columns = (int)arg_integer(args[3]);
    float tile_w = (float)arg_number(args[4]);
    float tile_h = (float)arg_number(args[5]);
    float ox = argc >= 7 ? (float)arg_number(args[6]) : 0.0f;
    float oy = argc >= 8 ? (float)arg_number(args[7]) : 0.0f;

    apply_physics(e, dt);
    e->grounded = 0;
    int hit_x = move_axis(e, e->vx * dt, 1,
                          map, columns, tile_w, tile_h, ox, oy);
    int hit_y = move_axis(e, e->vy * dt, 0,
                          map, columns, tile_w, tile_h, ox, oy);
    if (hit_x) e->vx = 0.0f;
    if (hit_y) {
        if (e->vy > 0.0f) e->grounded = 1;
        e->vy = 0.0f;
    }
    entity_advance_animation(e, dt);
    return gc_alloc_bool(vm, hit_x || hit_y);
}

/* ============================================================
 * Sprite / animação por entidade
 * ============================================================ */
static LvObject *native_entity_sprite(LvVM *vm, int argc, LvObject **args) {
    if (argc < 2 || !arg_string(args[1])) return gc_alloc_bool(vm, 0);
    LvEntity *e = entity_find((int)arg_integer(args[0]));
    if (!e) return gc_alloc_bool(vm, 0);

    const char *path = arg_string(args[1]);
    entity_release_sprite(e);
    e->texture = game_load_texture(path);
    if (!e->texture) return gc_alloc_bool(vm, 0);

    strncpy(e->sprite_path, path, sizeof(e->sprite_path) - 1);
    e->sprite_path[sizeof(e->sprite_path) - 1] = '\0';
    e->sprite_enabled = 1;
    e->sprite_frame_w = argc >= 3 ? (float)arg_number(args[2]) : (float)e->texture->width;
    e->sprite_frame_h = argc >= 4 ? (float)arg_number(args[3]) : (float)e->texture->height;
    e->sprite_frames = argc >= 5 ? (int)arg_integer(args[4]) : 1;
    e->sprite_fps = argc >= 6 ? (float)arg_number(args[5]) : 0.0f;
    e->sprite_row = argc >= 7 ? (int)arg_integer(args[6]) : 0;
    e->sprite_scale = argc >= 8 ? (float)arg_number(args[7]) : 1.0f;
    e->sprite_time = 0.0f;
    e->sprite_flip_x = 0;
    e->sprite_playing = 1;

    if (e->sprite_frame_w <= 0.0f) e->sprite_frame_w = (float)e->texture->width;
    if (e->sprite_frame_h <= 0.0f) e->sprite_frame_h = (float)e->texture->height;
    if (e->sprite_frames < 1) e->sprite_frames = 1;
    if (e->sprite_fps < 0.0f) e->sprite_fps = 0.0f;
    if (e->sprite_scale <= 0.0f) e->sprite_scale = 1.0f;
    if (e->sprite_row < 0) e->sprite_row = 0;

    return gc_alloc_bool(vm, 1);
}

static LvObject *native_entity_sprite_remove(LvVM *vm, int argc, LvObject **args) {
    LvEntity *e = argc >= 1 ? entity_find((int)arg_integer(args[0])) : NULL;
    if (!e) return gc_alloc_bool(vm, 0);
    entity_release_sprite(e);
    return gc_alloc_bool(vm, 1);
}

static LvObject *native_entity_sprite_flip(LvVM *vm, int argc, LvObject **args) {
    LvEntity *e = argc >= 2 ? entity_find((int)arg_integer(args[0])) : NULL;
    if (!e) return gc_alloc_bool(vm, 0);
    e->sprite_flip_x = arg_truthy(args[1]);
    return gc_alloc_bool(vm, 1);
}

static LvObject *native_entity_animation_play(LvVM *vm, int argc, LvObject **args) {
    LvEntity *e = argc >= 2 ? entity_find((int)arg_integer(args[0])) : NULL;
    if (!e || !e->sprite_enabled) return gc_alloc_bool(vm, 0);
    e->sprite_playing = arg_truthy(args[1]);
    if (argc >= 3 && arg_truthy(args[2])) e->sprite_time = 0.0f;
    return gc_alloc_bool(vm, 1);
}

static LvObject *native_entity_animation_frame(LvVM *vm, int argc, LvObject **args) {
    LvEntity *e = argc >= 1 ? entity_find((int)arg_integer(args[0])) : NULL;
    return gc_alloc_int(vm, e ? entity_animation_frame(e) : 0);
}

static LvObject *native_entity_draw_sprite(LvVM *vm, int argc, LvObject **args) {
    LvEntity *e = argc >= 1 ? entity_find((int)arg_integer(args[0])) : NULL;
    if (!e || !e->sprite_enabled || !e->texture) return vm->nil_singleton;

    int frame = entity_animation_frame(e);
    float src_w = e->sprite_flip_x ? -e->sprite_frame_w : e->sprite_frame_w;
    LvRect src = {
        frame * e->sprite_frame_w,
        e->sprite_row * e->sprite_frame_h,
        src_w,
        e->sprite_frame_h
    };

    float draw_w = e->sprite_frame_w * e->sprite_scale;
    float draw_h = e->sprite_frame_h * e->sprite_scale;
    LvRect dst = {e->x, e->y, draw_w, draw_h};

    int r = argc >= 2 ? clamp_byte((int)arg_integer(args[1])) : 255;
    int g = argc >= 3 ? clamp_byte((int)arg_integer(args[2])) : 255;
    int b = argc >= 4 ? clamp_byte((int)arg_integer(args[3])) : 255;
    int a = argc >= 5 ? clamp_byte((int)arg_integer(args[4])) : 255;

    game_draw_texture(e->texture, src, dst,
                      (LvColor){(uint8_t)r, (uint8_t)g,
                                (uint8_t)b, (uint8_t)a});
    return gc_alloc_int(vm, frame);
}

static LvObject *native_entity_draw(LvVM *vm, int argc, LvObject **args) {
    LvEntity *e = argc >= 4 ? entity_find((int)arg_integer(args[0])) : NULL;
    if (!e) return vm->nil_singleton;

    int r = clamp_byte((int)arg_integer(args[1]));
    int g = clamp_byte((int)arg_integer(args[2]));
    int b = clamp_byte((int)arg_integer(args[3]));
    int a = argc >= 5 ? clamp_byte((int)arg_integer(args[4])) : 255;

    game_draw_rect((LvRect){e->x, e->y, e->width, e->height},
                   (LvColor){(uint8_t)r, (uint8_t)g,
                             (uint8_t)b, (uint8_t)a});
    return vm->nil_singleton;
}

/* ============================================================
 * Getters / componentes
 * ============================================================ */
static LvObject *native_entity_x(LvVM *vm, int argc, LvObject **args) {
    LvEntity *e = argc >= 1 ? entity_find((int)arg_integer(args[0])) : NULL;
    return gc_alloc_float(vm, e ? e->x : 0.0);
}

static LvObject *native_entity_y(LvVM *vm, int argc, LvObject **args) {
    LvEntity *e = argc >= 1 ? entity_find((int)arg_integer(args[0])) : NULL;
    return gc_alloc_float(vm, e ? e->y : 0.0);
}

static LvObject *native_entity_vx(LvVM *vm, int argc, LvObject **args) {
    LvEntity *e = argc >= 1 ? entity_find((int)arg_integer(args[0])) : NULL;
    return gc_alloc_float(vm, e ? e->vx : 0.0);
}

static LvObject *native_entity_vy(LvVM *vm, int argc, LvObject **args) {
    LvEntity *e = argc >= 1 ? entity_find((int)arg_integer(args[0])) : NULL;
    return gc_alloc_float(vm, e ? e->vy : 0.0);
}

static LvObject *native_entity_grounded(LvVM *vm, int argc, LvObject **args) {
    LvEntity *e = argc >= 1 ? entity_find((int)arg_integer(args[0])) : NULL;
    return gc_alloc_bool(vm, e ? e->grounded : 0);
}

static LvObject *native_entity_name(LvVM *vm, int argc, LvObject **args) {
    LvEntity *e = argc >= 1 ? entity_find((int)arg_integer(args[0])) : NULL;
    const char *name = e ? e->name : "";
    return gc_alloc_string(vm, name, strlen(name));
}

static LvObject *native_entity_has_component(LvVM *vm, int argc, LvObject **args) {
    if (argc < 2 || !arg_string(args[1])) return gc_alloc_bool(vm, 0);
    LvEntity *e = entity_find((int)arg_integer(args[0]));
    if (!e) return gc_alloc_bool(vm, 0);

    const char *name = arg_string(args[1]);
    int has = 0;
    if (strcmp(name, "transform") == 0) has = 1;
    else if (strcmp(name, "fisica") == 0) has = e->physics_enabled;
    else if (strcmp(name, "colisor") == 0) has = e->collider_enabled;
    else if (strcmp(name, "sprite") == 0) has = e->sprite_enabled;
    else if (strcmp(name, "gatilho") == 0) has = e->trigger_enabled;
    return gc_alloc_bool(vm, has);
}

static LvObject *native_entity_count(LvVM *vm, int argc, LvObject **args) {
    (void)argc;
    (void)args;
    int count = 0;
    for (int i = 0; i < LVA_WORLD_ENTITY_MAX; i++)
        if (g_entities[i].active) count++;
    return gc_alloc_int(vm, count);
}

/* ============================================================
 * Mapas externos
 * ============================================================ */
static LvObject *native_map_load(LvVM *vm, int argc, LvObject **args) {
    if (argc < 1 || !arg_string(args[0])) return vm->nil_singleton;
    FILE *f = fopen(arg_string(args[0]), "r");
    if (!f) {
        fprintf(stderr, "[LEVIATA][MAPA] Nao foi possivel abrir: %s\n",
                arg_string(args[0]));
        return vm->nil_singleton;
    }

    LvObject *list = gc_alloc_list(vm);
    vm_push(vm, list);
    char line[8192];
    int width = 0;
    int height = 0;
    int failed = 0;

    while (fgets(line, sizeof(line), f)) {
        char *comment = strchr(line, '#');
        if (comment) *comment = '\0';

        int only_space = 1;
        for (char *p = line; *p; p++) {
            if (!isspace((unsigned char)*p)) {
                only_space = 0;
                break;
            }
        }
        if (only_space) continue;

        for (char *p = line; *p; p++) {
            if (*p == ',' || *p == ';' || *p == '\t') *p = ' ';
        }

        char *p = line;
        int row_count = 0;
        while (*p) {
            while (*p && isspace((unsigned char)*p)) p++;
            if (!*p) break;

            char *end = NULL;
            long value = strtol(p, &end, 10);
            if (end == p) {
                failed = 1;
                break;
            }

            LvObject *v = gc_alloc_int(vm, value);
            if (!list_push(list, v)) {
                failed = 1;
                break;
            }
            row_count++;
            p = end;
        }

        if (failed) break;
        if (row_count == 0) continue;
        if (width == 0) width = row_count;
        else if (row_count != width) {
            failed = 1;
            break;
        }
        height++;
    }

    fclose(f);
    vm_pop(vm);

    if (failed || width <= 0 || height <= 0) {
        fprintf(stderr,
                "[LEVIATA][MAPA] Formato invalido ou linhas com larguras diferentes.\n");
        return vm->nil_singleton;
    }

    g_last_map_width = width;
    g_last_map_height = height;
    return list;
}

static LvObject *native_map_width(LvVM *vm, int argc, LvObject **args) {
    (void)argc;
    (void)args;
    return gc_alloc_int(vm, g_last_map_width);
}

static LvObject *native_map_height(LvVM *vm, int argc, LvObject **args) {
    (void)argc;
    (void)args;
    return gc_alloc_int(vm, g_last_map_height);
}

/* ============================================================
 * Shutdown / registro
 * ============================================================ */
static LvObject *native_world_game_close(LvVM *vm, int argc, LvObject **args) {
    (void)argc;
    (void)args;
    camera_end();
    entity_release_all_sprites();
    leviata_2d_shutdown();
    game_close();
    return vm->nil_singleton;
}

void leviata_world_register_runtime(LvVM *vm) {
    if (!vm) return;

    /* Camera avançada: sobrescreve a câmera do módulo 2D. */
    vm_register_native(vm, "jogo_camera_configurar", native_camera_configure, -1);
    vm_register_native(vm, "jogo_camera_seguir", native_camera_follow, 2);
    vm_register_native(vm, "jogo_camera_suavidade", native_camera_smoothing, -1);
    vm_register_native(vm, "jogo_camera_iniciar", native_camera_begin, 0);
    vm_register_native(vm, "jogo_camera_finalizar", native_camera_end, 0);
    vm_register_native(vm, "jogo_camera_limites", native_camera_bounds, 4);
    vm_register_native(vm, "jogo_camera_limites_limpar", native_camera_bounds_clear, 0);

    /* Entidades e gameplay. */
    vm_register_native(vm, "entidade_criar", native_entity_create, 5);
    vm_register_native(vm, "entidade_destruir", native_entity_destroy, 1);
    vm_register_native(vm, "entidade_buscar", native_entity_find_by_name, 1);
    vm_register_native(vm, "entidade_posicionar", native_entity_position, 3);
    vm_register_native(vm, "entidade_tamanho", native_entity_size, 3);
    vm_register_native(vm, "entidade_velocidade", native_entity_velocity, 3);
    vm_register_native(vm, "entidade_fisica", native_entity_physics, -1);
    vm_register_native(vm, "entidade_colisor", native_entity_collider, 2);
    vm_register_native(vm, "entidade_gatilho", native_entity_trigger, 2);
    vm_register_native(vm, "entidade_colide", native_entity_collision, 2);
    vm_register_native(vm, "entidade_distancia", native_entity_distance, 2);
    vm_register_native(vm, "entidade_atualizar", native_entity_update, 2);
    vm_register_native(vm, "entidade_atualizar_tilemap", native_entity_update_tilemap, -1);

    /* Renderização de entidade. */
    vm_register_native(vm, "entidade_sprite", native_entity_sprite, -1);
    vm_register_native(vm, "entidade_sprite_remover", native_entity_sprite_remove, 1);
    vm_register_native(vm, "entidade_sprite_inverter_x", native_entity_sprite_flip, 2);
    vm_register_native(vm, "entidade_animacao_reproduzir", native_entity_animation_play, -1);
    vm_register_native(vm, "entidade_animacao_frame", native_entity_animation_frame, 1);
    vm_register_native(vm, "entidade_desenhar_sprite", native_entity_draw_sprite, -1);
    vm_register_native(vm, "entidade_desenhar", native_entity_draw, -1);

    /* Getters / introspecção. */
    vm_register_native(vm, "entidade_x", native_entity_x, 1);
    vm_register_native(vm, "entidade_y", native_entity_y, 1);
    vm_register_native(vm, "entidade_vx", native_entity_vx, 1);
    vm_register_native(vm, "entidade_vy", native_entity_vy, 1);
    vm_register_native(vm, "entidade_no_chao", native_entity_grounded, 1);
    vm_register_native(vm, "entidade_nome", native_entity_name, 1);
    vm_register_native(vm, "entidade_tem_componente", native_entity_has_component, 2);
    vm_register_native(vm, "entidade_quantidade", native_entity_count, 0);

    /* Mapas. */
    vm_register_native(vm, "mapa_carregar", native_map_load, 1);
    vm_register_native(vm, "mapa_largura", native_map_width, 0);
    vm_register_native(vm, "mapa_altura", native_map_height, 0);

    vm_register_native(vm, "jogo_fechar", native_world_game_close, 0);
}

void leviata_world_shutdown(void) {
    camera_end();
    entity_release_all_sprites();
    memset(g_entities, 0, sizeof(g_entities));
    g_next_entity_id = 1;
    g_last_map_width = 0;
    g_last_map_height = 0;
    memset(&g_camera, 0, sizeof(g_camera));
    g_camera.zoom = 1.0f;
}
