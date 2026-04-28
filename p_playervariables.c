/// @brief All contents that controls the player.

#include <stdio.h>
#include <math.h>


typedef struct {
    int zlook;
    u_int8_t max_zlook;
    float dirX;
    float dirY;
    float planeX;
    float planeY;
    int initfov;
    float fovratio;
    float fov_mult;
} P_PlayerState;

double fovratio;

P_PlayerState P_InitializePlayerState() 
{
    P_PlayerState p;
    
    p.zlook = 0;
    p.max_zlook = 255;

    p.initfov = 90;
    p.fovratio = tan(fabs(p.initfov/2.0f));

    p.dirX = -1.0;
    p.dirY = 0.0;
    p.planeX = 0.0;
    p.planeY = fovratio;
    
    p.fov_mult = 1;

    return p;
}