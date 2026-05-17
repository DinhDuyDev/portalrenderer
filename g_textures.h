/// Texture function headers

#ifndef G_TEXTURES_H
#define G_TEXTURES_H

int     G_LoadTextureIntoBitmapForm(char* filename, u_int32_t** pixels);
void    G_PrintBitmapData(u_int32_t**, int w, int h);

u_int32_t   G_MakeARGB32BitFormat(u_int32_t r, u_int32_t g, u_int32_t b, u_int32_t a);
void        G_GetARGBColor(u_int32_t color, u_int32_t* r, u_int32_t* g, u_int32_t* b, u_int32_t* a);

#endif