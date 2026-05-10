/* 
    Vector based 3d game.
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "SDL3/SDL.h"
#include "SDL3/SDL_video.h"
#include "SDL3/SDL_main.h"


#define W_WIDTH 640
#define W_HEIGHT 360
#define RAYCAST_SIZE_SCALE 320
#define TARGET_FPS 144

#define LEVEL_WIDTH 10
#define LEVEL_HEIGHT 10

#define tilesize 32
#define maxheight 10000
#define NEAR 1

#define TITLELENGTH 40

typedef struct {
    float x;
    float y;
} vec2d;

typedef struct {
    vec2d a;
    vec2d b;
    int z;
    int height;
} linedef;

double lerp(double x, double target, double weight) {
    return x + (target - x) * weight;
}

double deg2rad(double x) {
    return x * (M_PI / 180);
}

bool rect_collide(SDL_FRect *a, SDL_FRect *b) {
    if (a->x + a->w >= b->x && a->x <= b->x + b->w && a->y + a->h >= b->y && a->y <= b->y + b->h) {
        return true;
    }
    return false;
}

/* translates real world coordinates into screen coordinates */
vec2d M_WorldToScreen(float vx, float vy, int w) { 
    vec2d res;
    double transformY = vx;
    double transformX = w/2 * (1 + vy/(vx+0.1));
    res.x = transformX;
    res.y = transformY;
    return res;
}

linedef maplines[] = {
    {{-32, -32}, {0, -64}}, // AB
    {{0, -64}, {48, -32}}, // BC
    {{48, -32}, {16, 32}}, // CD
    {{16, 32}, {-16, 48}}, // DE
    {{-16, 48}, {-48, 16}}, // FE
    {{-48, 16}, {-32, -32}}, // EA
    {{0, 0}, {0, 32}, 0, -24},
    {{0, 32}, {0, 64}, 0, 16},
    // {{0, 64}, {0, 96}, 0, 24},
    // {{0, 96}, {0, 128}, 0, 32},
};
int maplineN = 6;

double normalize_direction(double x) {
    bool norm = true;
    while (norm) {
        if (x < 0) x += 360;
        else if (x >= 360) x -= 360;
        if (x >= 0 || x < 360) {norm = false;}
    }
    return x;
}

float vlen_squared(vec2d v) {
    return v.x * v.x + v.y * v.y;
}

/* compares two lines */
int linecomp(const void *a, const void *b) {
    // return ((linedef*) a)->a.y - ((linedef*)b)->a.y;
    linedef* l_a = (linedef*) a;
    linedef* l_b = (linedef*) b;
    
    int nearest_depth_a = (l_a->a.x + l_a->b.x) / 2; //(l_a->a.y < l_a->b.y) ? l_a->a.y : l_a->b.y;
    int nearest_depth_b = (l_b->a.x + l_b->b.x) / 2; //(l_b->a.y < l_b->b.y) ? l_b->a.y : l_b->b.y;

    return -(nearest_depth_a - nearest_depth_b);
}

/* always renders from right to left*/
void R_RenderWall(u_int32_t *pixbuff, u_int32_t *texture, int pnum, int tnum, int w, int h, int x1, int x2, int y11, int y12, int y21, int y22, linedef original, linedef modified)
{
    /* 
        If the height is out of the screen, then pray
    */

    int x_ptr = x1;
    int width = x2 - x1;
    double y0 = y11;
    double y1 = y12;
    int dy1 = y21 - y11;
    int dy2 = y22 - y12;

    /* length of obscured wall */
    float scale_to_original = 1.0 - (fabs(original.a.x - original.b.x) - fabs(modified.a.x - modified.b.x)) / (original.a.x - original.b.x);

    /* interpolation */
    while (x_ptr < x2) {
        /* if in bounds, then render */
        if (x_ptr >= 0 && x_ptr < w) {
            /* strips of wall */
            float ratio = (float) (x_ptr - x1) / width;
            y0 = y11 + ratio * dy1;
            y1 = y12 + ratio * dy2;
            u_int32_t col;
            for (int i = y0; i <= y1; i ++) {
                if (i >= 0 && i < h) {
                    int tx = scale_to_original * 288 + ((float) (x2 - x_ptr) / width - scale_to_original) * 288;
                    int ty = ((float) (i - y0) / (y1 - y0)) * 288;
                    if (fabs(i-y0) < 3 || fabs(i-y1) < 3 || x_ptr == x1 || x_ptr == x2-1) col = 0xffff00ff;
                    else col = 0x00000000;
                    pixbuff[i * pnum + x_ptr] = col;
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

int main(void) {

    /* window */
    char *windowtitle = (char*) malloc(sizeof(char) * TITLELENGTH);
    
    snprintf(windowtitle, TITLELENGTH, "FPS: %d", 0);

    /* game variables */
    bool running = true;

    /* camera */
    float playerX = -32;
    float playerY = -32;
    float playerZ = 64;
    int playerWidth = 16;
    float hsp = 0;
    float vsp = 0;
    float playerZlook = 0;
    int maxZlookup = 255;
    double direction = 0;

    int resolution = 320;

    float dt;
    uint64_t cTick = 0;
    uint64_t lTick = 0;
    int frames = 0;

    uint32_t currentTick = 0;
    uint32_t lastTick = 0;

    /* mouse pointer */
    SDL_FRect ptr;
    ptr.x = 0;
    ptr.y = 0;
    ptr.w = 4;
    ptr.h = 4;

    /* SDL */
    int window_width, window_height;
    float window_width_ratio, window_height_ratio;
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    SDL_Texture *screen_texture;
    SDL_Surface *surface;


    /* screen buffer */
    u_int32_t *pixels;
    int pitch;
    int pnum;
    int renderWidth;
    int renderHeight;
    bool map = false;

    /* texture buffer */
    u_int32_t *texpixels;
    int texpitch;
    int texpnum;
    int texW, texH;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL Init failed!");
        return SDL_APP_FAILURE;
    }

    /* transform geometry */
    linedef transformedlines[maplineN];
    int transformedPtr = 0;
    
    window = SDL_CreateWindow("Raycaster Demo", W_WIDTH, W_HEIGHT, SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_Log("Failed to create window! Error: %s", SDL_GetError());
        SDL_Quit();
        return SDL_APP_FAILURE;
    }

    renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        SDL_Log("Failed to create renderer! Error: %s", SDL_GetError());
        SDL_Quit();
        return SDL_APP_FAILURE;
    }
    SDL_SetRenderVSync(renderer, 1); /* turns on VSync and caps the framerate to the monitor's refresh rate (60fps) */
    SDL_SetWindowRelativeMouseMode(window, true);

    SDL_SetDefaultTextureScaleMode(renderer, SDL_SCALEMODE_NEAREST);
    surface = SDL_LoadSurface("images/marathon_brick_texture.png");

    if (!surface) {
        SDL_Log("Failed to create surface! Error: %s", SDL_GetError());
        SDL_Quit();
        return SDL_APP_FAILURE;
    }

    /* ensures the texture has the correct format */
    SDL_Texture *temp_texture = SDL_CreateTextureFromSurface(renderer, surface);
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, temp_texture->w, temp_texture->h);
    if (!texture) {
        SDL_Log("Failed to create texture! Error: %s", SDL_GetError());
        SDL_Quit();
        return SDL_APP_FAILURE;
    }

    if (!SDL_LockTexture(texture, NULL, (void*) &texpixels, &texpitch)) {
        SDL_Log("Failed to lock normal texture! Error %s", SDL_GetError());
        SDL_Quit();
        return SDL_APP_FAILURE;
    } else {
        texpnum = (int) texpitch / sizeof(u_int32_t);
        printf("Size of texture buffer: %lu\n", sizeof(texpixels));
        printf("Width of texture: %lu\n", surface->pitch / sizeof(u_int32_t));
        printf("Width of target: %d\n", texpnum);
        u_int32_t* pixarr = (u_int32_t*) surface->pixels;
        for (int y = 0; y < 288; y ++) {
            for (int x = 0; x < 288; x ++) {
                texpixels[y * texpnum + x] = pixarr[y * (surface->pitch / sizeof(u_int32_t))+ x];
            }
        }
    }


    SDL_DestroySurface(surface);

    /* screen buffer */
    renderWidth = resolution;
    renderHeight = W_HEIGHT;

    bool filled[renderWidth * renderHeight];

    screen_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, renderWidth, renderHeight);
    if (!screen_texture) {
        SDL_Log("Failed to create screen texture! Error: %s", SDL_GetError());
        SDL_Quit();
        return SDL_APP_FAILURE;
    } else {
        texpnum = (int) texpitch / sizeof(u_int32_t);
    }

    if (SDL_LockTexture(screen_texture, NULL, (void*) &pixels, &pitch)) {
        pnum = pitch / sizeof(u_int32_t);
        printf("Size of pixel buffer: %lu\n", sizeof(pixels));
        printf("Pitch value of texture buffer: %d\n", pitch);
    } else {
        SDL_Log("Failed to lock screen texture! Error: %s", SDL_GetError());
        SDL_Quit();
        return SDL_APP_FAILURE;
    }

    /* running */
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    while (running) {

        lastTick = currentTick;
        currentTick = SDL_GetTicks();
        dt = (currentTick - lastTick) / 100.0f;
        cTick = SDL_GetTicks();

        // SDL_GetWindowSizeInPixels(window, &window_width, &window_height);

        // window_width_ratio = (float) window_width / W_WIDTH;
        // window_height_ratio = (float) window_height / W_HEIGHT;

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            } else if (event.type == SDL_EVENT_KEY_DOWN) {
                if (event.key.key == SDLK_ESCAPE) {
                    running = false;
                } else if (event.key.key == SDLK_RETURN) {
                    SDL_SetWindowRelativeMouseMode(window, !SDL_GetWindowRelativeMouseMode(window));
                } else if (event.key.key == SDLK_M) {
                    map = !map;
                }
            } else if (event.type == SDL_EVENT_MOUSE_MOTION) {
                double mousexmove = -event.motion.xrel/10;
                double mouseymove = event.motion.yrel;
                direction = normalize_direction(direction - mousexmove);
                playerZlook += mouseymove;
                ptr.x = event.motion.x;
                ptr.y = event.motion.y;
            }
        }


        /* logic */
        double c = cos(deg2rad(-direction));
        double s = sin(deg2rad(-direction));
        for (int i = 0; i < maplineN; i ++) {
            linedef line = maplines[i];
            line.a.x -= playerX;
            line.a.y -= playerY;
            line.b.x -= playerX;
            line.b.y -= playerY;

            /* coords a */
            double ax = line.a.x;
            line.a.x = c * line.a.x - s * line.a.y;
            line.a.y = s * ax + c * line.a.y;

            /* coords b */
            double bx = line.b.x;
            line.b.x = c * line.b.x - s * line.b.y;
            line.b.y = s * bx + c * line.b.y;

            transformedlines[i] = line;
        }

        /* capping */
        if (playerZlook > maxZlookup) {
            playerZlook = maxZlookup;
        } 
        if (playerZlook < -maxZlookup) {
            playerZlook = -maxZlookup;
        }

        const bool *key_states = SDL_GetKeyboardState(NULL);

        int forward = (key_states[SDL_SCANCODE_W] - key_states[SDL_SCANCODE_S]);
        int sides   = (key_states[SDL_SCANCODE_A] - key_states[SDL_SCANCODE_D]);
        hsp = lerp(hsp, forward * cos(deg2rad(direction)) - sides * cos(deg2rad(direction+90)), 0.3);
        vsp = lerp(vsp, forward * sin(deg2rad(direction)) - sides * sin(deg2rad(direction+90)), 0.3);

        if (key_states[SDL_SCANCODE_E]) {
            playerZlook = 0;
        }

        /* collisions */
        playerX += hsp;
        playerY += vsp;

        /* plane maths */

        /* rendering */
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        if (SDL_LockTexture(screen_texture, NULL, (void*) &pixels, &pitch)) {
            pnum = pitch / sizeof(u_int32_t);
        }   else {
            SDL_Log("Failed to lock screen texture! Error: %s", SDL_GetError());
            running = false;
        }
        
        for (int i = 0; i < renderWidth * renderHeight; i ++) {
            pixels[i] = 0x0000;
            filled[i] = false;
        }

        double posX = playerX / tilesize;
        double posY = playerY / tilesize;

        int tex_width = texture->w;
        int tex_height = texture->h;

        /* drawing maplines */
        qsort(transformedlines, maplineN, sizeof(linedef), linecomp);
        for (int i = 0; i < maplineN; i ++) {
            linedef line = transformedlines[i];
            // if (line.a.x > line.b.x) continue;
            if (map) {
                SDL_FRect origin;
                origin.x = window_width/2-2;
                origin.y = window_height/2-2;
                origin.w = 4;
                origin.h = 4;
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                SDL_RenderLine(renderer, window_width/2 + line.a.y, window_height/2 - line.a.x, window_width/2 + line.b.y, window_height/2 - line.b.x);
                SDL_RenderRect(renderer, &origin);

                if (line.a.y > line.b.y) {
                    vec2d temp = line.a;
                    line.a = line.b;
                    line.b = temp;
                }

                if (line.a.x <= NEAR) {
                    double gradient = (double) ((line.b.x - line.a.x) / (line.b.y - line.a.y));
                    double x_onscreen = line.b.y - (double) ((line.b.x - NEAR) / gradient);
                    line.a.y = x_onscreen;
                    line.a.x = NEAR;

                    origin.x = window_width/2 + line.a.y;
                    origin.y = window_height/2;
                    
                    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
                    SDL_RenderRect(renderer, &origin);
                    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                }

                if (line.b.x <= NEAR) {
                    double gradient = (double) ((line.a.x - line.b.x) / (line.a.y - line.b.y));
                    double x_onscreen = line.a.y - (double) ((line.a.x - NEAR) / gradient);
                    line.b.y = x_onscreen;
                    line.b.x = NEAR;

                    origin.x = window_width/2 + line.b.y;
                    origin.y = window_height/2;

                    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
                    SDL_RenderRect(renderer, &origin);
                    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                }

            } else {
                if (line.a.x > NEAR || line.b.x > NEAR) {

                    linedef original = line;

                    if (line.a.x <= NEAR) {
                        double gradient = (double) (line.b.x - line.a.x) / (line.b.y - line.a.y);
                        double x_onscreen = line.b.y - (double) ((line.b.x - NEAR) / gradient);
                        line.a.y = x_onscreen;
                        line.a.x = NEAR;
                    }

                    if (line.b.x <= NEAR) {
                        double gradient = (double) (line.a.x - line.b.x) / (line.a.y - line.b.y);
                        double x_onscreen = line.a.y - (double) ((line.a.x - NEAR) / gradient);
                        line.b.y = x_onscreen;
                        line.b.x = NEAR;
                    }

                    vec2d pA = M_WorldToScreen(line.a.x, line.a.y, renderWidth);
                    vec2d pB = M_WorldToScreen(line.b.x, line.b.y, renderWidth);

                    if (pA.x > pB.x) {
                        vec2d temp = pA;
                        pA = pB;
                        pB = temp;
                    }
                    
                    if (pA.x > 0 || pB.x > 0 || pA.x < renderWidth || pB.x < 0) {
                        double heightA = renderHeight / (pA.y / 25 + 0.01);
                        double heightB = renderHeight / (pB.y / 25 + 0.01);
                        
                        double x1, x2, y11, y12, y21, y22;
                        x1 = pA.x;
                        x2 = pB.x;

                        // if (line.a.x < line.b.x) continue;

                        y11 = renderHeight/2 - (heightA/2) * (line.height / 32.0f) - playerZlook - (heightA/2) * (line.z / 32.0f);
                        y12 = renderHeight/2 + heightA/2 - playerZlook - (heightA/2) * (line.z / 32.0f);

                        y21 = renderHeight/2 - (heightB/2) * (line.height / 32.0f) - playerZlook - (heightB/2) * (line.z / 32.0f);
                        y22 = renderHeight/2 + heightB/2 - playerZlook - (heightB/2) * (line.z / 32.0f);

                        if (x1 > x2) {
                            double temp = x1;
                            x1 = x2;
                            x2 = temp;

                            temp = y11;
                            y11 = y21;
                            y21 = temp;

                            temp = y12;
                            y12 = y22;
                            y22 = temp;
                        }

                        R_RenderWall(pixels, texpixels, pnum, texpnum, renderWidth, renderHeight, x1, x2, y11, y12, y21, y22, original, line);
                    }
                }
            }
        }

        if (map) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
            SDL_RenderLine(renderer, 0, window_height/2, window_width, window_height/2);
        }


        frames ++;
        uint64_t elapsedTick = SDL_GetTicks() - cTick;
        if (cTick > lTick + 1000) {
            lTick = cTick;
            snprintf(windowtitle, TITLELENGTH, "Raycaster FPS: %d, x: %d, y: %d", frames, (int) playerX, (int) playerY);
            SDL_SetWindowTitle(window, windowtitle);
            frames = 0;
        }

        if (!map) {
            SDL_FRect screen_dst;
            screen_dst.x = window_width/2;
            screen_dst.y = window_height/2 - window_height/4;
            screen_dst.w = window_width/2;
            screen_dst.h = window_height/2;
            SDL_UnlockTexture(screen_texture);
            SDL_RenderTexture(renderer, screen_texture, NULL, NULL);
        }
        SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
        SDL_RenderPresent(renderer);
    }

    /* getting renderer name */
    SDL_Log("Renderer name: %s\n", SDL_GetRendererName(renderer));

    /* freeing everything */
    free(windowtitle);
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_UnlockTexture(texture);
    SDL_DestroyTexture(texture);
    SDL_DestroyTexture(temp_texture);
    SDL_UnlockTexture(screen_texture);
    SDL_DestroyTexture(screen_texture);
    SDL_Quit();

}