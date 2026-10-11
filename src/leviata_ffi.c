/* ============================================================================
 * MOTOR LEVIATA - FFI / BACKEND DE PLATAFORMA
 *
 * O build padrao continua sem dependencias graficas. Quando compilado com
 * LVA_USE_RAYLIB, este arquivo ativa janela, renderizacao 2D, entrada, texturas
 * e audio reais usando Raylib.
 * ============================================================================ */
#include "leviata_ffi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef LVA_USE_RAYLIB
#include <raylib.h>
#endif

/* ============================================================
 * Estado compartilhado do backend
 * ============================================================ */
static int   g_game_initialized = 0;
static int   g_game_width = 800;
static int   g_game_height = 600;
static float g_game_dt = 1.0f / 60.0f;
static int   g_stub_frame_count = 0;

#define LVA_TEXTURE_CACHE_MAX 128
static LvTexture *g_texture_cache[LVA_TEXTURE_CACHE_MAX];
static int g_texture_cache_count = 0;

#ifdef LVA_USE_RAYLIB
typedef struct {
    LvTexture public_texture;
    Texture2D native_texture;
} LvRayTexture;

static Sound g_current_sound;
static int   g_sound_loaded = 0;
static Music g_current_music;
static int   g_music_loaded = 0;
#endif

static LvColor lv_color(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
    LvColor c = { r, g, b, a };
    return c;
}

static int clamp_byte(int value) {
    if (value < 0) return 0;
    if (value > 255) return 255;
    return value;
}

static void texture_cache_clear(void);

/* ============================================================
 * FFI - JANELA / UI
 * ============================================================ */
LvWindow *ffi_window_create(const char *title, int width, int height) {
    LvWindow *win = (LvWindow*)calloc(1, sizeof(LvWindow));
    if (!win) return NULL;

    strncpy(win->title, title ? title : "Leviata", sizeof(win->title) - 1);
    win->title[sizeof(win->title) - 1] = '\0';
    win->width = width;
    win->height = height;
    win->platform_handle = NULL;
    win->is_open = 1;

#ifdef LVA_USE_RAYLIB
    if (!IsWindowReady()) {
        InitWindow(width, height, win->title);
    } else {
        SetWindowTitle(win->title);
        SetWindowSize(width, height);
    }
    SetTargetFPS(60);
#elif defined(LVA_USE_SDL2)
    printf("[FFI-SDL2] Backend SDL2 ainda nao implementado.\n");
#else
    printf("[FFI-Stub] Janela simulada: '%s' %dx%d\n", win->title, width, height);
#endif

    return win;
}

void ffi_window_destroy(LvWindow *win) {
    if (!win) return;
#ifdef LVA_USE_RAYLIB
    if (IsWindowReady()) CloseWindow();
#endif
    win->is_open = 0;
    free(win);
}

int ffi_window_should_close(LvWindow *win) {
    if (!win || !win->is_open) return 1;
#ifdef LVA_USE_RAYLIB
    return WindowShouldClose();
#else
    return 0;
#endif
}

void ffi_window_begin_frame(LvWindow *win) {
    if (!win || !win->is_open) return;
#ifdef LVA_USE_RAYLIB
    BeginDrawing();
#else
    (void)win;
#endif
}

void ffi_window_end_frame(LvWindow *win) {
    if (!win || !win->is_open) return;
#ifdef LVA_USE_RAYLIB
    EndDrawing();
#else
    (void)win;
#endif
}

void ffi_window_set_title(LvWindow *win, const char *title) {
    if (!win || !title) return;
    strncpy(win->title, title, sizeof(win->title) - 1);
    win->title[sizeof(win->title) - 1] = '\0';
#ifdef LVA_USE_RAYLIB
    if (IsWindowReady()) SetWindowTitle(win->title);
#endif
}

/* ============================================================
 * FFI - ENTRADA
 * ============================================================ */
void ffi_input_poll(LvInputState *state) {
    if (!state) return;
    memset(state, 0, sizeof(LvInputState));
#ifdef LVA_USE_RAYLIB
    for (int k = 0; k < 512; k++) state->keys[k] = IsKeyDown(k) ? 1 : 0;
    state->mouse_x = GetMouseX();
    state->mouse_y = GetMouseY();
    state->mouse_btn_left = IsMouseButtonDown(MOUSE_BUTTON_LEFT) ? 1 : 0;
    state->mouse_btn_right = IsMouseButtonDown(MOUSE_BUTTON_RIGHT) ? 1 : 0;
    int ch = GetCharPressed();
    state->last_char = (ch >= 0 && ch <= 255) ? (char)ch : '\0';
#endif
}

int ffi_key_down(LvInputState *state, int keycode) {
    if (!state || keycode < 0 || keycode >= 512) return 0;
    return state->keys[keycode];
}

/* ============================================================
 * GAME ENGINE - LOOP
 * ============================================================ */
int game_init(const char *title, int width, int height, int fps) {
    if (fps <= 0) fps = 60;
    if (width <= 0) width = 800;
    if (height <= 0) height = 600;

    g_game_width = width;
    g_game_height = height;
    g_game_dt = 1.0f / (float)fps;
    g_stub_frame_count = 0;
    g_game_initialized = 1;

#ifdef LVA_USE_RAYLIB
    if (!IsWindowReady()) {
        InitWindow(width, height, title ? title : "Leviata");
    } else {
        SetWindowTitle(title ? title : "Leviata");
        SetWindowSize(width, height);
    }
    SetTargetFPS(fps);
#elif defined(LVA_USE_SDL2)
    printf("[GAME-SDL2] Backend SDL2 ainda nao implementado.\n");
#else
    printf("[GAME-Stub] '%s' %dx%d @ %d FPS\n",
           title ? title : "Leviata", width, height, fps);
    printf("[GAME-Stub] Use 'make raylib' para o backend grafico real.\n");
#endif

    return 1;
}

void game_close(void) {
    if (!g_game_initialized) return;

    texture_cache_clear();

#ifdef LVA_USE_RAYLIB
    if (g_sound_loaded) {
        StopSound(g_current_sound);
        UnloadSound(g_current_sound);
        g_sound_loaded = 0;
    }
    if (g_music_loaded) {
        StopMusicStream(g_current_music);
        UnloadMusicStream(g_current_music);
        g_music_loaded = 0;
    }
    if (IsAudioDeviceReady()) CloseAudioDevice();
    if (IsWindowReady()) CloseWindow();
#endif

    g_game_initialized = 0;
}

int game_should_close(void) {
    if (!g_game_initialized) return 1;
#ifdef LVA_USE_RAYLIB
    return WindowShouldClose();
#else
    /* Evita loop infinito quando um jogo for executado no build sem backend. */
    return g_stub_frame_count > 0;
#endif
}

void game_begin_frame(void) {
    if (!g_game_initialized) return;
#ifdef LVA_USE_RAYLIB
    BeginDrawing();
#endif
}

void game_end_frame(void) {
    if (!g_game_initialized) return;
#ifdef LVA_USE_RAYLIB
    if (g_music_loaded) UpdateMusicStream(g_current_music);
    if (g_sound_loaded && !IsSoundPlaying(g_current_sound)) {
        UnloadSound(g_current_sound);
        g_sound_loaded = 0;
    }
    EndDrawing();
    g_game_dt = GetFrameTime();
#else
    g_stub_frame_count++;
#endif
}

float game_get_delta_time(void) {
#ifdef LVA_USE_RAYLIB
    if (IsWindowReady()) return GetFrameTime();
#endif
    return g_game_dt;
}

float game_get_fps(void) {
#ifdef LVA_USE_RAYLIB
    if (IsWindowReady()) return (float)GetFPS();
#endif
    return g_game_dt > 0.0f ? 1.0f / g_game_dt : 0.0f;
}

/* ============================================================
 * GAME ENGINE - RENDERIZACAO 2D
 * ============================================================ */
void game_clear_background(LvColor color) {
#ifdef LVA_USE_RAYLIB
    ClearBackground((Color){ color.r, color.g, color.b, color.a });
#else
    (void)color;
#endif
}

void game_draw_rect(LvRect rect, LvColor color) {
#ifdef LVA_USE_RAYLIB
    DrawRectangleRec((Rectangle){ rect.x, rect.y, rect.width, rect.height },
                     (Color){ color.r, color.g, color.b, color.a });
#else
    (void)rect; (void)color;
#endif
}

void game_draw_rect_outline(LvRect rect, LvColor color, float thick) {
#ifdef LVA_USE_RAYLIB
    DrawRectangleLinesEx((Rectangle){ rect.x, rect.y, rect.width, rect.height },
                         thick,
                         (Color){ color.r, color.g, color.b, color.a });
#else
    (void)rect; (void)color; (void)thick;
#endif
}

void game_draw_circle(float cx, float cy, float radius, LvColor color) {
#ifdef LVA_USE_RAYLIB
    DrawCircleV((Vector2){ cx, cy }, radius,
                (Color){ color.r, color.g, color.b, color.a });
#else
    (void)cx; (void)cy; (void)radius; (void)color;
#endif
}

void game_draw_text(const char *text, float x, float y, int size, LvColor color) {
#ifdef LVA_USE_RAYLIB
    DrawText(text ? text : "", (int)x, (int)y, size,
             (Color){ color.r, color.g, color.b, color.a });
#else
    (void)text; (void)x; (void)y; (void)size; (void)color;
#endif
}

void game_draw_texture(LvTexture *tex, LvRect src, LvRect dst, LvColor tint) {
    if (!tex) return;
#ifdef LVA_USE_RAYLIB
    LvRayTexture *impl = (LvRayTexture*)tex;
    if (impl->native_texture.id == 0) return;

    if (src.width <= 0.0f) src.width = (float)impl->native_texture.width;
    if (src.height <= 0.0f) src.height = (float)impl->native_texture.height;
    if (dst.width <= 0.0f) dst.width = src.width;
    if (dst.height <= 0.0f) dst.height = src.height;

    DrawTexturePro(impl->native_texture,
                   (Rectangle){ src.x, src.y, src.width, src.height },
                   (Rectangle){ dst.x, dst.y, dst.width, dst.height },
                   (Vector2){ 0.0f, 0.0f }, 0.0f,
                   (Color){ tint.r, tint.g, tint.b, tint.a });
#else
    (void)src; (void)dst; (void)tint;
#endif
}

void game_draw_line(float x1, float y1, float x2, float y2, LvColor color, float thick) {
#ifdef LVA_USE_RAYLIB
    DrawLineEx((Vector2){ x1, y1 }, (Vector2){ x2, y2 }, thick,
               (Color){ color.r, color.g, color.b, color.a });
#else
    (void)x1; (void)y1; (void)x2; (void)y2; (void)color; (void)thick;
#endif
}

/* ============================================================
 * GAME ENGINE - TEXTURAS
 * ============================================================ */
LvTexture *game_load_texture(const char *path) {
    if (!path) return NULL;
#ifdef LVA_USE_RAYLIB
    LvRayTexture *impl = (LvRayTexture*)calloc(1, sizeof(LvRayTexture));
    if (!impl) return NULL;

    impl->native_texture = LoadTexture(path);
    if (impl->native_texture.id == 0) {
        free(impl);
        return NULL;
    }

    impl->public_texture.id = (int)impl->native_texture.id;
    impl->public_texture.width = impl->native_texture.width;
    impl->public_texture.height = impl->native_texture.height;
    strncpy(impl->public_texture.path, path, sizeof(impl->public_texture.path) - 1);
    impl->public_texture.path[sizeof(impl->public_texture.path) - 1] = '\0';
    return &impl->public_texture;
#else
    LvTexture *t = (LvTexture*)calloc(1, sizeof(LvTexture));
    if (!t) return NULL;
    t->id = 1;
    t->width = 64;
    t->height = 64;
    strncpy(t->path, path, sizeof(t->path) - 1);
    t->path[sizeof(t->path) - 1] = '\0';
    return t;
#endif
}

void game_unload_texture(LvTexture *tex) {
    if (!tex) return;
#ifdef LVA_USE_RAYLIB
    LvRayTexture *impl = (LvRayTexture*)tex;
    if (impl->native_texture.id != 0) UnloadTexture(impl->native_texture);
    free(impl);
#else
    free(tex);
#endif
}

static LvTexture *texture_cache_get(const char *path) {
    if (!path) return NULL;
    for (int i = 0; i < g_texture_cache_count; i++) {
        if (g_texture_cache[i] && strcmp(g_texture_cache[i]->path, path) == 0)
            return g_texture_cache[i];
    }

    if (g_texture_cache_count >= LVA_TEXTURE_CACHE_MAX) {
        fprintf(stderr, "[LEVIATA][GAME] Cache de texturas cheio (%d).\n", LVA_TEXTURE_CACHE_MAX);
        return NULL;
    }

    LvTexture *tex = game_load_texture(path);
    if (!tex) {
        fprintf(stderr, "[LEVIATA][GAME] Nao foi possivel carregar textura: %s\n", path);
        return NULL;
    }
    g_texture_cache[g_texture_cache_count++] = tex;
    return tex;
}

static void texture_cache_clear(void) {
    for (int i = 0; i < g_texture_cache_count; i++) {
        game_unload_texture(g_texture_cache[i]);
        g_texture_cache[i] = NULL;
    }
    g_texture_cache_count = 0;
}

/* ============================================================
 * GAME ENGINE - ENTRADA
 * ============================================================ */
int game_key_down(LvKeyCode key) {
#ifdef LVA_USE_RAYLIB
    return IsKeyDown((int)key) ? 1 : 0;
#else
    (void)key; return 0;
#endif
}

int game_key_pressed(LvKeyCode key) {
#ifdef LVA_USE_RAYLIB
    return IsKeyPressed((int)key) ? 1 : 0;
#else
    (void)key; return 0;
#endif
}

int game_key_released(LvKeyCode key) {
#ifdef LVA_USE_RAYLIB
    return IsKeyReleased((int)key) ? 1 : 0;
#else
    (void)key; return 0;
#endif
}

int game_mouse_button_down(int button) {
#ifdef LVA_USE_RAYLIB
    return IsMouseButtonDown(button) ? 1 : 0;
#else
    (void)button; return 0;
#endif
}

float game_mouse_x(void) {
#ifdef LVA_USE_RAYLIB
    return (float)GetMouseX();
#else
    return 0.0f;
#endif
}

float game_mouse_y(void) {
#ifdef LVA_USE_RAYLIB
    return (float)GetMouseY();
#else
    return 0.0f;
#endif
}

/* ============================================================
 * GAME ENGINE - AUDIO
 * ============================================================ */
int game_init_audio(void) {
#ifdef LVA_USE_RAYLIB
    if (!IsAudioDeviceReady()) InitAudioDevice();
    return IsAudioDeviceReady() ? 1 : 0;
#else
    return 1;
#endif
}

void game_play_sound(const char *path) {
    if (!path) return;
#ifdef LVA_USE_RAYLIB
    if (!IsAudioDeviceReady()) InitAudioDevice();
    if (g_sound_loaded) {
        StopSound(g_current_sound);
        UnloadSound(g_current_sound);
        g_sound_loaded = 0;
    }
    g_current_sound = LoadSound(path);
    g_sound_loaded = 1;
    PlaySound(g_current_sound);
#else
    printf("[GAME-Stub] som: %s\n", path);
#endif
}

void game_play_music(const char *path) {
    if (!path) return;
#ifdef LVA_USE_RAYLIB
    if (!IsAudioDeviceReady()) InitAudioDevice();
    if (g_music_loaded) {
        StopMusicStream(g_current_music);
        UnloadMusicStream(g_current_music);
        g_music_loaded = 0;
    }
    g_current_music = LoadMusicStream(path);
    g_music_loaded = 1;
    PlayMusicStream(g_current_music);
#else
    printf("[GAME-Stub] musica: %s\n", path);
#endif
}

void game_set_volume(float vol) {
    if (vol < 0.0f) vol = 0.0f;
    if (vol > 1.0f) vol = 1.0f;
#ifdef LVA_USE_RAYLIB
    if (!IsAudioDeviceReady()) InitAudioDevice();
    SetMasterVolume(vol);
#else
    (void)vol;
#endif
}

/* ============================================================
 * BINDINGS PT -> FFI REAL
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
    if (o->type == LV_TYPE_INT) return o->as.i_val;
    if (o->type == LV_TYPE_FLOAT) return (int64_t)o->as.f_val;
    return 0;
}

static const char *arg_string(LvObject *o) {
    return (o && o->type == LV_TYPE_STRING) ? o->as.string.data : NULL;
}

static LvObject *native_game_start(LvVM *vm, int argc, LvObject **args) {
    if (argc < 3 || !arg_string(args[0]) || !arg_is_number(args[1]) || !arg_is_number(args[2])) {
        fprintf(stderr, "[LEVIATA][GAME] jogo_iniciar(titulo, largura, altura, fps?)\n");
        return gc_alloc_bool(vm, 0);
    }
    int fps = (argc >= 4 && arg_is_number(args[3])) ? (int)arg_integer(args[3]) : 60;
    return gc_alloc_bool(vm, game_init(arg_string(args[0]), (int)arg_integer(args[1]),
                                       (int)arg_integer(args[2]), fps));
}

static LvObject *native_game_close(LvVM *vm, int argc, LvObject **args) {
    (void)argc; (void)args;
    game_close();
    return vm->nil_singleton;
}

static LvObject *native_game_should_close(LvVM *vm, int argc, LvObject **args) {
    (void)argc; (void)args;
    return gc_alloc_bool(vm, game_should_close());
}

static LvObject *native_game_begin_frame(LvVM *vm, int argc, LvObject **args) {
    (void)argc; (void)args;
    game_begin_frame();
    return vm->nil_singleton;
}

static LvObject *native_game_end_frame(LvVM *vm, int argc, LvObject **args) {
    (void)argc; (void)args;
    game_end_frame();
    return vm->nil_singleton;
}

static LvObject *native_game_clear(LvVM *vm, int argc, LvObject **args) {
    if (argc < 3) return vm->nil_singleton;
    int a = argc >= 4 ? (int)arg_integer(args[3]) : 255;
    game_clear_background(lv_color((unsigned char)clamp_byte((int)arg_integer(args[0])),
                                   (unsigned char)clamp_byte((int)arg_integer(args[1])),
                                   (unsigned char)clamp_byte((int)arg_integer(args[2])),
                                   (unsigned char)clamp_byte(a)));
    return vm->nil_singleton;
}

static LvObject *native_game_rect(LvVM *vm, int argc, LvObject **args) {
    if (argc < 7) return vm->nil_singleton;
    LvRect r = { (float)arg_number(args[0]), (float)arg_number(args[1]),
                 (float)arg_number(args[2]), (float)arg_number(args[3]) };
    int a = argc >= 8 ? (int)arg_integer(args[7]) : 255;
    game_draw_rect(r, lv_color((unsigned char)clamp_byte((int)arg_integer(args[4])),
                               (unsigned char)clamp_byte((int)arg_integer(args[5])),
                               (unsigned char)clamp_byte((int)arg_integer(args[6])),
                               (unsigned char)clamp_byte(a)));
    return vm->nil_singleton;
}

static LvObject *native_game_circle(LvVM *vm, int argc, LvObject **args) {
    if (argc < 6) return vm->nil_singleton;
    int a = argc >= 7 ? (int)arg_integer(args[6]) : 255;
    game_draw_circle((float)arg_number(args[0]), (float)arg_number(args[1]),
                     (float)arg_number(args[2]),
                     lv_color((unsigned char)clamp_byte((int)arg_integer(args[3])),
                              (unsigned char)clamp_byte((int)arg_integer(args[4])),
                              (unsigned char)clamp_byte((int)arg_integer(args[5])),
                              (unsigned char)clamp_byte(a)));
    return vm->nil_singleton;
}

static LvObject *native_game_text(LvVM *vm, int argc, LvObject **args) {
    if (argc < 7 || !arg_string(args[0])) return vm->nil_singleton;
    int a = argc >= 8 ? (int)arg_integer(args[7]) : 255;
    game_draw_text(arg_string(args[0]), (float)arg_number(args[1]), (float)arg_number(args[2]),
                   (int)arg_integer(args[3]),
                   lv_color((unsigned char)clamp_byte((int)arg_integer(args[4])),
                            (unsigned char)clamp_byte((int)arg_integer(args[5])),
                            (unsigned char)clamp_byte((int)arg_integer(args[6])),
                            (unsigned char)clamp_byte(a)));
    return vm->nil_singleton;
}

static LvObject *native_game_sprite(LvVM *vm, int argc, LvObject **args) {
    if (argc < 3 || !arg_string(args[0])) return vm->nil_singleton;
    LvTexture *tex = texture_cache_get(arg_string(args[0]));
    if (!tex) return vm->nil_singleton;

    float w = argc >= 4 ? (float)arg_number(args[3]) : (float)tex->width;
    float h = argc >= 5 ? (float)arg_number(args[4]) : (float)tex->height;
    LvRect src = { 0.0f, 0.0f, (float)tex->width, (float)tex->height };
    LvRect dst = { (float)arg_number(args[1]), (float)arg_number(args[2]), w, h };
    game_draw_texture(tex, src, dst, lv_color(255, 255, 255, 255));
    return vm->nil_singleton;
}

static LvObject *native_game_key_down(LvVM *vm, int argc, LvObject **args) {
    if (argc < 1) return gc_alloc_bool(vm, 0);
    return gc_alloc_bool(vm, game_key_down((LvKeyCode)arg_integer(args[0])));
}

static LvObject *native_game_key_pressed(LvVM *vm, int argc, LvObject **args) {
    if (argc < 1) return gc_alloc_bool(vm, 0);
    return gc_alloc_bool(vm, game_key_pressed((LvKeyCode)arg_integer(args[0])));
}

static LvObject *native_game_key_released(LvVM *vm, int argc, LvObject **args) {
    if (argc < 1) return gc_alloc_bool(vm, 0);
    return gc_alloc_bool(vm, game_key_released((LvKeyCode)arg_integer(args[0])));
}

static LvObject *native_game_mouse_button(LvVM *vm, int argc, LvObject **args) {
    if (argc < 1) return gc_alloc_bool(vm, 0);
    return gc_alloc_bool(vm, game_mouse_button_down((int)arg_integer(args[0])));
}

static LvObject *native_game_mouse_x(LvVM *vm, int argc, LvObject **args) {
    (void)argc; (void)args;
    return gc_alloc_float(vm, game_mouse_x());
}

static LvObject *native_game_mouse_y(LvVM *vm, int argc, LvObject **args) {
    (void)argc; (void)args;
    return gc_alloc_float(vm, game_mouse_y());
}

static LvObject *native_game_delta(LvVM *vm, int argc, LvObject **args) {
    (void)argc; (void)args;
    return gc_alloc_float(vm, game_get_delta_time());
}

static LvObject *native_game_fps(LvVM *vm, int argc, LvObject **args) {
    (void)argc; (void)args;
    return gc_alloc_float(vm, game_get_fps());
}

static LvObject *native_game_audio_init(LvVM *vm, int argc, LvObject **args) {
    (void)argc; (void)args;
    return gc_alloc_bool(vm, game_init_audio());
}

static LvObject *native_game_sound(LvVM *vm, int argc, LvObject **args) {
    if (argc >= 1 && arg_string(args[0])) game_play_sound(arg_string(args[0]));
    return vm->nil_singleton;
}

static LvObject *native_game_music(LvVM *vm, int argc, LvObject **args) {
    if (argc >= 1 && arg_string(args[0])) game_play_music(arg_string(args[0]));
    return vm->nil_singleton;
}

static LvObject *native_game_volume(LvVM *vm, int argc, LvObject **args) {
    if (argc >= 1) game_set_volume((float)arg_number(args[0]));
    return vm->nil_singleton;
}

void ffi_register_runtime(LvVM *vm) {
    if (!vm) return;

    vm_register_native(vm, "jogo_iniciar",          native_game_start,        -1);
    vm_register_native(vm, "jogo_fechar",           native_game_close,         0);
    vm_register_native(vm, "jogo_deve_fechar",      native_game_should_close,  0);
    vm_register_native(vm, "jogo_iniciar_frame",    native_game_begin_frame,   0);
    vm_register_native(vm, "jogo_finalizar_frame",  native_game_end_frame,     0);
    vm_register_native(vm, "jogo_limpar_fundo",     native_game_clear,        -1);
    vm_register_native(vm, "jogo_desenhar_sprite",  native_game_sprite,       -1);
    vm_register_native(vm, "jogo_tecla",            native_game_key_down,      1);
    vm_register_native(vm, "jogo_mouse_x",          native_game_mouse_x,       0);
    vm_register_native(vm, "jogo_mouse_y",          native_game_mouse_y,       0);
    vm_register_native(vm, "jogo_delta",            native_game_delta,         0);

    vm_register_native(vm, "jogo_desenhar_retangulo", native_game_rect,        -1);
    vm_register_native(vm, "jogo_desenhar_circulo",   native_game_circle,      -1);
    vm_register_native(vm, "jogo_desenhar_texto",     native_game_text,        -1);
    vm_register_native(vm, "jogo_tecla_apertada",     native_game_key_pressed,  1);
    vm_register_native(vm, "jogo_tecla_solta",        native_game_key_released, 1);
    vm_register_native(vm, "jogo_mouse_botao",        native_game_mouse_button, 1);
    vm_register_native(vm, "jogo_fps",                native_game_fps,          0);
    vm_register_native(vm, "jogo_audio_iniciar",      native_game_audio_init,   0);
    vm_register_native(vm, "jogo_tocar_som",          native_game_sound,        1);
    vm_register_native(vm, "jogo_tocar_musica",       native_game_music,        1);
    vm_register_native(vm, "jogo_volume",             native_game_volume,       1);
}
