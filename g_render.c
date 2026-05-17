/// @brief: DRAWING FUNCTIONS DEFINITIONS


#include <stdio.h>
#include "g_render.h"
#include "w_geometry.h"
#include <math.h>

/* RENDERING PIPELINE:
    1. Translate one vector.
    2. See if a portal is in the viewport.
    3. See 
*/

void R_RenderWall(u_int32_t *pixbuff,  int* zbuffer, int pnum, int w, int h, int x1, int x2, int y11, int y12, int y21, int y22, linedef modified, int top, int bottom, int left, int right)
{
    /*
        If the height is out of the screen, then pray
        x is now the distance to the projection plane.
    */

    int x_ptr = x1;
    int width = x2 - x1;
    double y0 = y11;
    double y1 = y12;
    int dy1 = y21 - y11;
    int dy2 = y22 - y12;

    /* interpolation */
    while (x_ptr < x2) {
        /* if in bounds, then render */
        if (x_ptr >= 0 && x_ptr < w) {
            /* strips of wall */
            float ratio = (float) (x_ptr - x1) / width;
            y0 = y11 + ratio * dy1;
            y1 = y12 + ratio * dy2;
            u_int32_t col;

            int dist_from_projection = fabsf(modified.a.x + ((modified.b.x - modified.a.x) * ratio));

            for (int i = y0; i <= y1; i ++) {
                if (i >= 0 && i < h) {
                    if (zbuffer[i * pnum + x_ptr] >= dist_from_projection) {
                    int t, b, l, r;
                    t = fabs(i-y0) < 3 && top;
                    b = fabs(i-y1) < 3 && bottom;
                    l = x_ptr == x1 && left;
                    r = x_ptr == x2-1 && right;
                    if (t || b || l || r) col = (modified.isPortal) ? 0xff00ffff :  0xffff00ff;
                    else col = 0x00000000;
                    pixbuff[i * pnum + x_ptr] = col;
                    zbuffer[i * pnum + x_ptr] = dist_from_projection;
                    }
                } else {
                    if (i >= h) {
                        break;
                    }
                    if (i < 0) i = 0;
                }
            }
        } else {
            if (x_ptr < 0) {
                x_ptr = 0;
            }
        }
        x_ptr ++;
    }
}

int R_WallSegmentVisible(int* zbuffer, int pnum, int w, int h, wallsegment segment)
{
    /*
        If the height is out of the screen, then pray
        x is now the distance to the projection plane.
    */

    int x_ptr = segment.x1;
    int width = segment.x2 - segment.x1;
    double y0 = segment.y11;
    double y1 = segment.y12;
    int dy1 = segment.y21 - segment.y11;
    int dy2 = segment.y22 - segment.y12;

    int portalpixelcount = 0;

    /* interpolation */
    while (x_ptr < segment.x2) {
        /* if in bounds, then render */
        if (x_ptr >= 0 && x_ptr < w) {
            /* strips of segment */
            float ratio = (float) (x_ptr - segment.x1) / width;
            y0 = segment.y11 + ratio * dy1;
            y1 = segment.y12 + ratio * dy2;

            for (int i = y0; i <= y1; i ++) {
                if (i >= 0 && i < h) {
                    if (zbuffer[i * pnum + x_ptr] >= 9998) {
                        portalpixelcount ++;
                    }
                } else {
                    if (i >= h) {
                        break;
                    }
                    if (i < 0) i = 0;
                }
            }
        } else {
            if (x_ptr < 0) {
                x_ptr = 0;
            }
        }
        x_ptr ++;
    }
    if (portalpixelcount > 2048) {
        printf("portal is seen\n");
        return 1;
    } else
        printf("--------->\n");
    return 0;
}
