/* ============================================================================
 * MOTOR LEVIATA - WORLD / ENTIDADES / FISICA
 * Entidades leves, componentes, movimento, mapas por arquivo e camera limitada.
 * ============================================================================ */
#ifndef LEVIATA_WORLD_H
#define LEVIATA_WORLD_H

#include "leviata_vm.h"

#define LVA_WORLD_ENTITY_MAX 1024

typedef struct {
    int id;
    int active;
    char name[128];
    float x, y;
    float width, height;
    float vx, vy;
    float gravity;
    float drag;
    int physics_enabled;
    int collider_enabled;
    int grounded;
} LvEntity;

void leviata_world_register_runtime(LvVM *vm);
void leviata_world_shutdown(void);

#endif /* LEVIATA_WORLD_H */
