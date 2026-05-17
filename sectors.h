#ifndef SECTORS_H
#define SECTORS_H

#include "w_geometry.h"

/* getting all visible sectors */
void W_SequenceSector(int sector_id, sector sector_arr[], int *sector_count);
void W_TranslateSectorVectors(sector sectors_src[], sector sectors_dest[], int sector_id, int x, int y, double direction);

#endif