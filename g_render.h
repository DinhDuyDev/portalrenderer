//
//  DRAWING FUNCTIONS DECLARATION
//


// Include guard to prevent multiple definitions
#ifndef G_RENDER_H
#define G_RENDER_H

#include <stdio.h>
#include "sectors.h"

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
void R_RenderWall(u_int32_t *pixbuff, u_int8_t* filled, int pnum, int w, int h, int x1, int x2, int y11, int y12, int y21, int y22, int top, int bottom, int left, int right, linedef line);

/* seeing if a wall section is visible */
int R_PortalVisible(u_int8_t* filled, int pnum, int w, int h, wallsegment segment);

/* size of a wall segment */

int R_SegmentSize(int w, int h, wallsegment);

/* filling a wall segment*/
// void R_PortalizeRegion(int pnum, int w, int h, wallsegment segment);

/* queueing up all the walls */



#endif
