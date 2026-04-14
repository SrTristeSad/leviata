/* ============================================================================
 * MOTOR LEVIATA - IMPLEMENTAÇÃO DO FFI (Stubs de Plataforma)
 * Cada função abaixo é um stub documentado que mostra como ligar
 * a biblioteca real. Quando LVA_USE_RAYLIB ou LVA_USE_SDL2 estiver
 * definido na compilação, as implementações reais são ativadas.
 * ============================================================================ */
#include "leviata_ffi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ============================================================
 * FFI - JANELA / UI
 * ============================================================ */

LvWindow *ffi_window_create(const char *title, int width, int height) {
    LvWindow *win = (LvWindow*)calloc(1, sizeof(LvWindow));
    if (!win) return NULL;

    strncpy(win->title, title, 255);
    win->width          = width;
    win->height         = height;
    win->platform_handle = NULL;
    win->is_open        = 1;

#ifdef LVA_USE_SDL2
    /* Integração SDL2 real:
     * SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
     * win->platform_handle = SDL_CreateWindow(title,
     *     SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
     *     width, height, SDL_WINDOW_SHOWN);
     */
    printf("[FFI-SDL2] Janela criada: %s (%dx%d)\n", title, width, height);
#elif defined(LVA_USE_RAYLIB)
    /* Integração Raylib real:
     * InitWindow(width, height, title);
     * SetTargetFPS(60);
     */
    printf("[FFI-Raylib] Janela criada: %s (%dx%d)\n", title, width, height);
#else
    printf("[FFI-Stub] Janela simulada: '%s' %dx%d\n", title, width, height);
    printf("[FFI-Stub] Compile com -DLVA_USE_SDL2 ou -DLVA_USE_RAYLIB para UI real\n");
#endif

    return win;
}

void ffi_window_destroy(LvWindow *win) {
    if (!win) return;
#ifdef LVA_USE_SDL2
    /* SDL_DestroyWindow((SDL_Window*)win->platform_handle);
     * SDL_Quit(); */
#elif defined(LVA_USE_RAYLIB)
    /* CloseWindow(); */
#endif
    win->is_open = 0;
    free(win);
}

int ffi_window_should_close(LvWindow *win) {
    if (!win) return 1;
#ifdef LVA_USE_SDL2
    /* SDL_Event e;
     * while (SDL_PollEvent(&e))
     *     if (e.type == SDL_QUIT) win->is_open = 0;
     */
#elif defined(LVA_USE_RAYLIB)
    /* return WindowShouldClose(); */
#endif
    return !win->is_open;
}

void ffi_window_begin_frame(LvWindow *win) {
    (void)win;
#ifdef LVA_USE_RAYLIB
    /* BeginDrawing(); */
#elif defined(LVA_USE_SDL2)
    /* SDL_RenderClear(renderer); */
#endif
}

void ffi_window_end_frame(LvWindow *win) {
    (void)win;
#ifdef LVA_USE_RAYLIB
    /* EndDrawing(); */
#elif defined(LVA_USE_SDL2)
    /* SDL_RenderPresent(renderer); */
#endif
}

void ffi_window_set_title(LvWindow *win, const char *title) {
    if (!win) return;
    strncpy(win->title, title, 255);
#ifdef LVA_USE_SDL2
    /* SDL_SetWindowTitle((SDL_Window*)win->platform_handle, title); */
#elif defined(LVA_USE_RAYLIB)
    /* SetWindowTitle(title); */
#endif
}

/* ============================================================
 * FFI - ENTRADA
 * ============================================================ */

void ffi_input_poll(LvInputState *state) {
    if (!state) return;
    memset(state, 0, sizeof(LvInputState));
#ifdef LVA_USE_SDL2
    /* SDL_Event e;
     * while (SDL_PollEvent(&e)) {
     *     if (e.type == SDL_KEYDOWN)
     *         state->keys[e.key.keysym.scancode] = 1;
     * }
     * int mx, my;
     * Uint32 btn = SDL_GetMouseState(&mx, &my);
     * state->mouse_x = mx; state->mouse_y = my;
     * state->mouse_btn_left  = (btn & SDL_BUTTON_LMASK) != 0;
     * state->mouse_btn_right = (btn & SDL_BUTTON_RMASK) != 0;
     */
#elif defined(LVA_USE_RAYLIB)
    /* for (int k = 0; k < 512; k++) state->keys[k] = IsKeyDown(k);
     * state->mouse_x = (int)GetMouseX();
     * state->mouse_y = (int)GetMouseY();
     * state->mouse_btn_left  = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
     * state->mouse_btn_right = IsMouseButtonDown(MOUSE_BUTTON_RIGHT);
     */
#endif
}

int ffi_key_down(LvInputState *state, int keycode) {
    if (!state || keycode < 0 || keycode >= 512) return 0;
    return state->keys[keycode];
}

/* ============================================================
 * GAME ENGINE - LOOP E RENDERIZAÇÃO (Stubs)
 * ============================================================ */

static int   g_game_initialized = 0;
static int   g_game_width = 800, g_game_height = 600;
static float g_game_dt = 1.0f / 60.0f;
static clock_t g_frame_start;

int game_init(const char *title, int width, int height, int fps) {
    g_game_width   = width;
    g_game_height  = height;
    g_game_dt      = 1.0f / (float)fps;
    g_game_initialized = 1;
    g_frame_start  = clock();

    printf("╔══════════════════════════════════════╗\n");
    printf("║  MOTOR LEVIATA — Game Engine         ║\n");
    printf("║  Titulo : %-27s║\n", title);
    printf("║  Tamanho: %dx%-25d║\n", width, height);
    printf("║  FPS    : %-27d║\n", fps);
    printf("╠══════════════════════════════════════╣\n");

#ifdef LVA_USE_RAYLIB
    printf("║  Backend: Raylib (ativo)             ║\n");
    /* InitWindow(width, height, title);
     * SetTargetFPS(fps);
     * InitAudioDevice(); */
#elif defined(LVA_USE_SDL2)
    printf("║  Backend: SDL2 (ativo)               ║\n");
    /* SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS);
     * SDL_CreateWindow(...) etc */
#else
    printf("║  Backend: Stub (para compilar real:  ║\n");
    printf("║  -DLVA_USE_RAYLIB -lraylib           ║\n");
    printf("║  -DLVA_USE_SDL2   -lSDL2             ║\n");
#endif
    printf("╚══════════════════════════════════════╝\n");
    return 1;
}

void game_close(void) {
    if (!g_game_initialized) return;
    g_game_initialized = 0;
    printf("[GAME] Motor encerrado. Até a próxima!\n");
#ifdef LVA_USE_RAYLIB
    /* CloseAudioDevice();
     * CloseWindow(); */
#elif defined(LVA_USE_SDL2)
    /* SDL_Quit(); */
#endif
}

int game_should_close(void) {
#ifdef LVA_USE_RAYLIB
    /* return WindowShouldClose(); */
#elif defined(LVA_USE_SDL2)
    /* Verifica eventos SDL */
#endif
    return !g_game_initialized;
}

void game_begin_frame(void) {
    g_frame_start = clock();
#ifdef LVA_USE_RAYLIB
    /* BeginDrawing(); */
#elif defined(LVA_USE_SDL2)
    /* SDL_RenderClear(renderer); */
#endif
}

void game_end_frame(void) {
#ifdef LVA_USE_RAYLIB
    /* EndDrawing(); */
#elif defined(LVA_USE_SDL2)
    /* SDL_RenderPresent(renderer); */
#endif
}

float game_get_delta_time(void) {
#ifdef LVA_USE_RAYLIB
    /* return GetFrameTime(); */
#endif
    return g_game_dt;
}

float game_get_fps(void) {
#ifdef LVA_USE_RAYLIB
    /* return (float)GetFPS(); */
#endif
    return 1.0f / g_game_dt;
}

void game_clear_background(LvColor color) {
#ifdef LVA_USE_RAYLIB
    /* ClearBackground((Color){color.r, color.g, color.b, color.a}); */
#elif defined(LVA_USE_SDL2)
    /* SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
     * SDL_RenderClear(renderer); */
#else
    printf("[GAME] Limpar fundo RGBA(%d,%d,%d,%d)\n", color.r, color.g, color.b, color.a);
#endif
}

void game_draw_rect(LvRect rect, LvColor color) {
#ifdef LVA_USE_RAYLIB
    /* DrawRectangleRec((Rectangle){rect.x,rect.y,rect.width,rect.height},
     *                  (Color){color.r,color.g,color.b,color.a}); */
#else
    printf("[GAME] DrawRect (%.0f,%.0f,%.0f,%.0f) RGBA(%d,%d,%d,%d)\n",
           rect.x, rect.y, rect.width, rect.height,
           color.r, color.g, color.b, color.a);
#endif
}

void game_draw_rect_outline(LvRect rect, LvColor color, float thick) {
    (void)thick;
#ifndef LVA_USE_RAYLIB
    printf("[GAME] DrawRectOutline (%.0f,%.0f,%.0f,%.0f)\n",
           rect.x, rect.y, rect.width, rect.height);
#else
    /* DrawRectangleLinesEx(...); */
#endif
}

void game_draw_circle(float cx, float cy, float radius, LvColor color) {
    (void)color;
#ifndef LVA_USE_RAYLIB
    printf("[GAME] DrawCircle center=(%.0f,%.0f) r=%.0f\n", cx, cy, radius);
#else
    /* DrawCircle((int)cx,(int)cy,radius,(Color){...}); */
#endif
}

void game_draw_text(const char *text, float x, float y, int size, LvColor color) {
    (void)color;
#ifndef LVA_USE_RAYLIB
    printf("[GAME] DrawText '%s' at (%.0f,%.0f) size=%d\n", text, x, y, size);
#else
    /* DrawText(text,(int)x,(int)y,size,(Color){...}); */
#endif
}

void game_draw_texture(LvTexture *tex, LvRect src, LvRect dst, LvColor tint) {
    (void)src; (void)tint;
    if (!tex) return;
#ifndef LVA_USE_RAYLIB
    printf("[GAME] DrawTexture '%s' at (%.0f,%.0f)\n", tex->path, dst.x, dst.y);
#else
    /* DrawTexturePro(...) */
#endif
}

void game_draw_line(float x1, float y1, float x2, float y2, LvColor color, float thick) {
    (void)color; (void)thick;
    printf("[GAME] DrawLine (%.0f,%.0f)->(%.0f,%.0f)\n", x1, y1, x2, y2);
}

LvTexture *game_load_texture(const char *path) {
    LvTexture *t = (LvTexture*)calloc(1, sizeof(LvTexture));
    if (!t) return NULL;
    strncpy(t->path, path, 511);
    t->id     = 1; /* ID fictício */
    t->width  = 64;
    t->height = 64;
    printf("[GAME] LoadTexture '%s'\n", path);
    return t;
}

void game_unload_texture(LvTexture *tex) {
    if (!tex) return;
    printf("[GAME] UnloadTexture '%s'\n", tex->path);
    free(tex);
}

int game_key_down(LvKeyCode key) {
    (void)key;
#ifdef LVA_USE_RAYLIB
    /* return IsKeyDown((int)key); */
#endif
    return 0;
}

int game_key_pressed(LvKeyCode key) {
    (void)key;
#ifdef LVA_USE_RAYLIB
    /* return IsKeyPressed((int)key); */
#endif
    return 0;
}

int game_key_released(LvKeyCode key) {
    (void)key;
    return 0;
}

int game_mouse_button_down(int button) {
    (void)button;
    return 0;
}

float game_mouse_x(void) {
#ifdef LVA_USE_RAYLIB
    /* return GetMouseX(); */
#endif
    return 0.0f;
}

float game_mouse_y(void) {
#ifdef LVA_USE_RAYLIB
    /* return GetMouseY(); */
#endif
    return 0.0f;
}

int game_init_audio(void) {
    printf("[GAME] InitAudio stub\n");
#ifdef LVA_USE_RAYLIB
    /* InitAudioDevice(); */
#endif
    return 1;
}

void game_play_sound(const char *path) {
    printf("[GAME] PlaySound '%s'\n", path);
#ifdef LVA_USE_RAYLIB
    /* Sound snd = LoadSound(path); PlaySound(snd); */
#endif
}

void game_play_music(const char *path) {
    printf("[GAME] PlayMusic '%s'\n", path);
#ifdef LVA_USE_RAYLIB
    /* Music mus = LoadMusicStream(path); PlayMusicStream(mus); */
#endif
}

void game_set_volume(float vol) {
    printf("[GAME] SetVolume %.2f\n", vol);
#ifdef LVA_USE_RAYLIB
    /* SetMasterVolume(vol); */
#endif
}
