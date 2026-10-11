/* ============================================================================
 * MOTOR LEVIATA - MODULO 2D AVANCADO
 * Camera, colisao, spritesheet/animacao, tilemap e cenas.
 * ============================================================================ */
#ifndef LEVIATA_2D_H
#define LEVIATA_2D_H

#include "leviata_vm.h"

typedef struct {
    float target_x;
    float target_y;
    float offset_x;
    float offset_y;
    float rotation;
    float zoom;
} LvCameraState;

/* Colisao utilitaria disponivel tambem para codigo C hospedeiro. */
int leviata_2d_rects_collide(float ax, float ay, float aw, float ah,
                             float bx, float by, float bw, float bh);
int leviata_2d_point_in_rect(float px, float py,
                             float rx, float ry, float rw, float rh);
int leviata_2d_circles_collide(float ax, float ay, float ar,
                               float bx, float by, float br);

/* Registra a API PT do motor 2D por cima do runtime FFI base. */
void leviata_2d_register_runtime(LvVM *vm);

/* Libera caches e estado do modulo. Seguro chamar mais de uma vez. */
void leviata_2d_shutdown(void);

#endif /* LEVIATA_2D_H */
