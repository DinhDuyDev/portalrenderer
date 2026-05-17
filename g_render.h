//
//  DRAWING FUNCTIONS DECLARATION
//


// Include guard to prevent multiple definitions
#ifndef G_RENDER_H
#define G_RENDER_H

#include <stdio.h>
#include "w_geometry.h"

/* wall segment */
typedef struct {
    int portal_id;
    int x1;
    int x2;
    int y11;
    int y12;
    int y21;
    int y22;
} wallsegment;

/* drawing a horizontal trapezoidal wall */
void R_RenderWall(u_int32_t *pixbuff, int* zbuffer, int pnum, int w, int h, int x1, int x2, int y11, int y12, int y21, int y22, linedef modified, int top, int bottom, int left, int right);

/* seeing if a wall section is visible */
int R_WallSegmentVisible(int* zbuffer, int pnum, int w, int h, wallsegment segment);

/* queueing up all the walls */



#endif
