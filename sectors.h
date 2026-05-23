#ifndef SECTORS_H
#define SECTORS_H

#include <stdio.h>

#define MAXSECTORLINEDEF 35
#define MAXPORTALSINONESECTOR 30
#define O_GEOMETRY

typedef struct {
    float x;
    float y;
} vec2d;

typedef struct {
    vec2d a;
    vec2d b;
    int z1; // top height
    int z2; // bottom height
    int isPortal; // if it is a portal -> can be seen through
    int portalZ1; // upper boundary of the portal rendering space
    int portalZ2; // lower boundary of the portal rendering space
} linedef;


typedef struct {
    size_t num_linedef;
    size_t num_wall;
    int neighboring_sectors[MAXSECTORLINEDEF]; /* indices of neighboring sectors. must be the same indices as portal inds*/
    int portal_inds[MAXPORTALSINONESECTOR]; /* indices of all portals */
    int wall_inds[MAXSECTORLINEDEF]; /* indices of all walls */
    linedef lines[MAXSECTORLINEDEF]; /* flexible array members must be malloc'd */
} sector;

#endif