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

/* dot product of two vectors */
double dot(vec2d a, vec2d b) 
{
    return a.x * b.x + a.y * b.y;
}

/* squared length of two vectors */
double sqrlen(vec2d vector) 
{
    return dot(vector, vector);
}

vec2d projection(vec2d a, vec2d b) 
{
    vec2d res;
    double scale = (double) dot(a, b) / dot(b, b);
    res.x = b.x * scale;
    res.y = b.y * scale;
    return res;
}

vec2d ldeftovec(linedef line)
{
    return (vec2d) {line.a.x - line.b.x, line.a.y - line.b.y};
}

float squaredlength(int x1, int y1, int x2, int y2)
{
    return (x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1);
}

float dist(float x1, float y1, float x2, float y2)
{
    return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

/* 
    Taken from Jeffrey Thompson's Collision Detection: https://www.jeffreythompson.org/collision-detection/
*/
char linepoint(float x1, float y1, float x2, float y2, float px, float py) {

  // get distance from the point to the two ends of the line
  float d1 = dist(px,py, x1,y1);
  float d2 = dist(px,py, x2,y2);

  // get the length of the line
  float lineLen = dist(x1,y1, x2,y2);

  // since floats are so minutely accurate, add
  // a little buffer zone that will give collision
  float buffer = 0.1;    // higher # = less accurate

  // if the two distances are equal to the line's 
  // length, the point is on the line!
  // note we use the buffer here to give a range, 
  // rather than one #
  if (d1+d2 >= lineLen-buffer && d1+d2 <= lineLen+buffer) {
    return 1;
  }
  return 0;
}

/* 
    Adapted from Jeffrey Thompson's Collision Detection: https://www.jeffreythompson.org/collision-detection/
*/
char pointcircle(float x1, float y1, float x2, float y2, float cx, float cy, int radius) 
{
    /* circular collision */
    char inside1 = dist(cx, cy, x1, y1) <= radius;
    char inside2 = dist(cx, cy, x2, y2) <= radius;

    if (inside1 || inside2) return 1;

    float distX = x1 - x2;
    float distY = y1 - y2;
    float len = distX * distX + distY * distY;
    float dot = (float) ((cx - x1) * (x2 - x1) + (cy - y1) * (y2 - y1)) / len;

    float closestX = x1 + (dot * (x2 - x1));
    float closestY = y1 + (dot * (y2 - y1));

    char onsegment = linepoint(x1, y1, x2, y2, closestX, closestY);

    if (!onsegment) return 0;

    distX = closestX - cx;
    distY = closestY - cy;

    float distance = sqrt(distX * distX + distY * distY);

    return distance <= radius;
}