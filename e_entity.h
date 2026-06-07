/// @brief Entity definition

#ifndef E_ENTITY_H
#define E_ENTITY_H

#include "e_entitydefs.h"
#include "sectors.h"

static int id = 0;
typedef struct
{
    int id;             /* [PLAN]: 0 is player */
    float x;            /* x-coordinate     */
    float y;            /* y-coordinate     */
    float z;            /* z-coordinate     */
    float hsp;          /* horizontal speed */
    float vsp;          /* vertical speed   */
    int w;              /* width            */
    int h;              /* height           */
    E_EntityTag tag;    /* special tag      */

    double dist_to_walls[MAXSECTORLINEDEF]; /* all distances to walls */

} E_GameEntity;

E_GameEntity E_InitializeGameEntity(float x, float y, float z, int w, int h, E_EntityTag tag) {
    E_GameEntity ent;
    ent.x = x;
    ent.y = y;
    ent.z = z;
    ent.hsp = 0;
    ent.vsp = 0;
    ent.w = w;
    ent.h = h;
    ent.tag = tag;
    return ent;
}

#endif