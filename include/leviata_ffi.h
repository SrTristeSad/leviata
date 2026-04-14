/* ============================================================================
 * MOTOR LEVIATA - FFI (FOREIGN FUNCTION INTERFACE / BINDINGS)
 * Interface para chamar código C nativo a partir da linguagem PT.
 * ============================================================================ */
#ifndef LEVIATA_FFI_H
#define LEVIATA_FFI_H

#include "leviata_vm.h"

/* ============================================================
 * BINDING DE INTERFACE GRÁFICA (UI / Widgets)
 * Stub preparado para integração real com SDL2, GTK+, Win32 API, etc.
 * ============================================================ */
typedef struct {
    int   width;
    int   height;
    char  title[256];
    void *platform_handle;  /* ponteiro opaco para handle nativo */
    int   is_open;
} LvWindow;

typedef struct {
    int x, y;
    int width, height;
    char text[256];
    int  pressed;
} LvButton;

typedef struct {
    int  mouse_x, mouse_y;
    int  mouse_btn_left, mouse_btn_right;
    int  keys[512];     /* estado de cada tecla */
    char last_char;
} LvInputState;

/* ---- API de Janela (stub OSS) ---- */
LvWindow *ffi_window_create(const char *title, int width, int height);
void      ffi_window_destroy(LvWindow *win);
int       ffi_window_should_close(LvWindow *win);
void      ffi_window_begin_frame(LvWindow *win);
void      ffi_window_end_frame(LvWindow *win);
void      ffi_window_set_title(LvWindow *win, const char *title);

/* ---- API de Entrada ---- */
void ffi_input_poll(LvInputState *state);
int  ffi_key_down(LvInputState *state, int keycode);

/* ============================================================
 * BINDING DE MOTOR DE JOGOS (Game Engine)
 * Stub para integração com Raylib ou SDL2+OpenGL
 * ============================================================ */

/* Chaves do teclado (compatível com Raylib/SDL) */
typedef enum {
    LV_KEY_A = 65, LV_KEY_D = 68, LV_KEY_S = 83, LV_KEY_W = 87,
    LV_KEY_UP = 265, LV_KEY_DOWN = 264, LV_KEY_LEFT = 263, LV_KEY_RIGHT = 262,
    LV_KEY_SPACE = 32, LV_KEY_ENTER = 257, LV_KEY_ESC = 256
} LvKeyCode;

/* Cor RGBA simples */
typedef struct { uint8_t r, g, b, a; } LvColor;

/* Sprite / Textura */
typedef struct {
    int   id;         /* ID da textura (handle OpenGL/SDL) */
    int   width;
    int   height;
    char  path[512];
} LvTexture;

/* Retângulo */
typedef struct { float x, y, width, height; } LvRect;

/* ---- Game Loop API (stub) ---- */
int   game_init(const char *title, int width, int height, int fps);
void  game_close(void);
int   game_should_close(void);
void  game_begin_frame(void);
void  game_end_frame(void);
float game_get_delta_time(void);
float game_get_fps(void);

/* ---- Renderização 2D ---- */
void game_clear_background(LvColor color);
void game_draw_rect(LvRect rect, LvColor color);
void game_draw_rect_outline(LvRect rect, LvColor color, float thick);
void game_draw_circle(float cx, float cy, float radius, LvColor color);
void game_draw_text(const char *text, float x, float y, int size, LvColor color);
void game_draw_texture(LvTexture *tex, LvRect src, LvRect dst, LvColor tint);
void game_draw_line(float x1, float y1, float x2, float y2, LvColor color, float thick);

/* ---- Texturas ---- */
LvTexture *game_load_texture(const char *path);
void       game_unload_texture(LvTexture *tex);

/* ---- Entrada no Jogo ---- */
int   game_key_down(LvKeyCode key);
int   game_key_pressed(LvKeyCode key);
int   game_key_released(LvKeyCode key);
int   game_mouse_button_down(int button);
float game_mouse_x(void);
float game_mouse_y(void);

/* ---- Áudio (stub) ---- */
int  game_init_audio(void);
void game_play_sound(const char *path);
void game_play_music(const char *path);
void game_set_volume(float vol);

#endif /* LEVIATA_FFI_H */
