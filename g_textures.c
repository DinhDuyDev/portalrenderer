#include "SDL3/SDL.h"
#include "SDL3/SDL_video.h"
#include "SDL3/SDL_main.h"
#include <stdlib.h>
#include "g_textures.h"

//
//  TEXTURE FUNCTIONS DEFINITIONS
//


/* Returns pnum (pitch / bytes to store color) of texture. Takes a pointer to fill out the pixels data. */
int G_LoadTextureIntoBitmapForm(char* filename, u_int32_t** pixels) 
{
    SDL_Surface* temporary_surface = SDL_LoadSurface(filename);
    if (!temporary_surface) {
        SDL_Log("Failed to load texture '%s' as a surface. Error: %s\n", filename, SDL_GetError());
        return -1;
    }

    *pixels = (u_int32_t*) malloc(sizeof(**pixels) * temporary_surface->w * temporary_surface->h);

    if (!(*pixels)) {
        fprintf(stderr, "Error! Malloc for texture %s failed.\n", filename);
    } else {
        fprintf(stdout, "Texture allocation for %s successful! %d bits bytes assigned for texture.\n", filename, temporary_surface->w * temporary_surface->h);
    }

    u_int32_t* pixarr = (u_int32_t*) temporary_surface->pixels;
    int p = temporary_surface->pitch / sizeof(u_int32_t);
    for (int y = 0; y < temporary_surface->h; y ++) {
        for (int x = 0; x < temporary_surface->w; x ++) {
            (*pixels)[y * p + x] = pixarr[y * p + x];
        }
    }

    printf("Dimensions: %d, %d, pitch: %lu\n", temporary_surface->w, temporary_surface->h, p * sizeof(u_int32_t));
    SDL_DestroySurface(temporary_surface);
    return p * sizeof(u_int32_t);

}

/* Print out bitmap data in hex form. */
void G_PrintBitmapData(u_int32_t** pixels, int w, int h) 
{
    for (int y = 0; y < h; y ++) {
        printf("%i ", y);
        for (int x = 0; x < w; x ++) {
            printf("%i: %x, ", x, (*pixels)[y * w + x]);
        }
        printf("\n");
    }
}

/* Make a 32-bit integer from r, g, b, a to transcribe into RGBA */
u_int32_t G_MakeARGB32BitFormat(u_int32_t r, u_int32_t g, u_int32_t b, u_int32_t a) 
{
    u_int32_t red   = r << 16;
    u_int32_t green = g << 8;
    u_int32_t blue  = b;
    u_int32_t alpha = a << 24;
    return red | green | blue | alpha;
}

/* Get RGBA components from 32-bit integer and put them in r, g, b, a pointers. */
void G_GetARGBColor(u_int32_t col, u_int32_t* r, u_int32_t* g, u_int32_t* b, u_int32_t* a) {
    // *r = col >> 24;
    // *g = (col << 8) >> 24;
    // *b = (col << 16) >> 24;
    // *a = (col << 24) >> 24;
    *a = col >> 24;
    *r = (col << 8) >> 24;
    *g = (col << 16) >> 24;
    *b = (col << 24) >> 24;
}


// 00000000000000000000000000000000
// 10000000

// int main(void) 
// {
//     u_int32_t color = 0xffaaffff;
//     u_int32_t r, g, b, a;
//     G_GetARGBColor(color, &r, &g, &b, &a);
//     printf("%u %u %u %u\n", a - 10, r - 10, g - 10, b - 10);
//     u_int32_t colshade = G_MakeARGB32BitFormat(r - 10, g - 10, b - 10, a - 10);
//     G_GetARGBColor(colshade, &r, &g, &b, &a);
//     printf("%u %u %u %u\n", a, r, g, b);
//     return 0;
// }