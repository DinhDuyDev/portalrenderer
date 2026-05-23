/*
    ALL UTILITY FUNCTIONS WILL BE DEFINED HERE.
*/

#ifndef M_UTIL_H
#define M_UTIL_H

#include <math.h>
#include "sectors.h"

/* gradually moves x to target through a weight */
double lerp(double x, double target, double weight);
/* turns degrees into radians */
double deg2rad(double x);

/* keep the direction capped between 0 and 360 */
double normalize_direction(double x);

/* translates real world coordinates into screen coordinates */
vec2d M_WorldToScreen(float vx, float vy, int w);

#endif