/* ============================================================================
 * MOTOR LEVIATA - WORLD / ENTIDADES / FISICA
 * Entidades leves, componentes, movimento, mapas, sprites e gatilhos.
 * ============================================================================ */
#ifndef LEVIATA_WORLD_H
#define LEVIATA_WORLD_H

#include "leviata_vm.h"
#include "leviata_ffi.h"

#define LVA_WORLD_ENTITY_MAX 1024

typedef struct {
    int id;
    int active;
    char name[128];

    /* Transform / collider */
    float x, y;
    float width, height;

    /* Physics */
    float vx, vy;
    float gravity;
    float drag;
    int physics_enabled;
    int collider_enabled;
    int grounded;

    /* Trigger / gameplay */
    int trigger_enabled;

    /* Sprite / animation */
    int sprite_enabled;
    LvTexture *texture;
    char sprite_path[512];
    float sprite_frame_w;
    float sprite_frame_h;
    int sprite_frames;
    int sprite_row;
    float sprite_fps;
    float sprite_time;
    float sprite_scale;
    int sprite_flip_x;
    int sprite_playing;
} LvEntity;

void leviata_world_register_runtime(LvVM *vm);
void leviata_world_shutdown(void);

#endif /* LEVIATA_WORLD_H */
