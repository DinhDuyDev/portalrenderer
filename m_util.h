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

/* dot product */
double dot(vec2d a, vec2d b);

/* length of vector */
double sqrlen(vec2d vector);

/* return orthogonal projection of a onto b */
vec2d projection(vec2d a, vec2d b);

/* returns vec2d version of a linedef */
vec2d ldeftovec(linedef line);

/* returns squared length */
float squaredlength(int x1, int y1, int x2, int y2);

/* translates real world coordinates into screen coordinates */
vec2d M_WorldToScreen(float vx, float vy, int w);

/* 
    Adapted from Jeffrey Thompson's Collision Detection: https://www.jeffreythompson.org/collision-detection/
*/
char linepoint(float x1, float y1, float x2, float y2, float px, float py);

/* returns precise distance between two points */
float dist(float x1, float y1, float x2, float y2);

/*
    Adapted from Jeffrey Thompson's Collision Detection: https://www.jeffreythompson.org/collision-detection/
*/
char pointcircle(float x1, float y1, float x2, float y2, float cx, float cy, int radius);

#endif