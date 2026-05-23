#include <math.h>
#include "sectors.h"

/* gradually moves x to target through a weight */
double lerp(double x, double target, double weight) 
{
    return x + (target - x) * weight;
}

/* turns degrees into radians */
double deg2rad(double x) 
{
    return x * (M_PI / 180);
}

/* keep the direction capped between 0 and 360 */
double normalize_direction(double x) 
{
    int norm = 1;
    while (norm) {
        if (x < 0) x += 360;
        else if (x >= 360) x -= 360;
        if (x >= 0 || x < 360) {norm = 0;}
    }
    return x;
}

/* translates real world coordinates into screen coordinates */
vec2d M_WorldToScreen(float vx, float vy, int w) 
{ 
    vec2d res;
    double transformY = vx;
    double transformX = w/2 * (1 + vy/(vx+0.1));
    res.x = transformX;
    res.y = transformY;
    return res;
}