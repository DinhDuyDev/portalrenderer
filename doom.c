/* 
    Vector based 3d game.
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"

#include "g_gameparameters.h"
#include "m_util.h"
#include "e_entity.h"
#include "g_textures.h"
#include "w_geometry.h"

#include "g_render.h"
#include "p_playervariables.c"

#define MAXSECTORS 100
#define MAXLINEDEF 50
#define MAXVISSECTOR 5


linedef lines_arr[MAXLINEDEF];
int lines_arr_ln = 0;
int lines_arr_index = 0;

sector visible_sectors[MAXVISSECTOR];
int visible_sectors_ln = 0;
int visible_sectors_index = 0;

wallsegment portals[MAXPORTALSINONESECTOR];
int portals_ln = 0;
int portals_index = 0;

// a, b, z1, z2 isPortal, portalZ1, portalZ2
sector sectorslist[MAXSECTORS] = {
    {
        .num_linedef = 6,
        .num_wall = 5,
        .lines = {
            {{-32, -32}, {0, -64}, 32, 0, 0, 0, 0}, 
            {{0, -64}, {48, -32}, 32, 0, 0, 0, 0}, 
            {{48, -32}, {16, 32}, 32, 0, 0, 0, 0}, 
            {{16, 32}, {-16, 48}, 32, 0, 1, 24, 8}, 
            {{-16, 48}, {-48, 16}, 32, 0, 0, 0, 0}, 
            {{-48, 16}, {-32, -32}, 32, 0, 0, 0, 0},
        },
        .neighboring_sectors = {1},
        .corresponding_portal_id = {3},
        .walls_id = {0, 1, 2, 4, 5},
    },

    {
        .num_linedef = 4,
        .num_wall = 3,
        .lines = {
            {{16, 32}, {-16, 48}, 32, 0, 1, 24, 8},
            {{16, 32}, {32, 64}, 32, 0, 0, 0, 0},
            {{0, 80}, {32, 64}, 32, 0, 0, 0, 0},
            {{-16, 48}, {0, 80}, 32, 0, 0, 0, 0},
        },
        .neighboring_sectors = {0},
        .corresponding_portal_id = {0},
        .walls_id = {1, 2, 3},
    }
};


int main(void) {
    // sectorslist[1].lines[0] = sectorslist[0].lines[3];
    /* window */
    char* windowtitle = (char*) malloc(sizeof(char) * TITLELENGTH);
    
    snprintf(windowtitle, TITLELENGTH, "FPS: %d", 0);

    /* game variables */
    bool running = true;

    /* camera */
    P_PlayerState playerstate = P_InitializePlayerState();
    E_GameEntity playerentity = {.x = 0, .y = 0, .z = 0, .w = 16, .h = 16};

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
    SDL_Texture *screen_texture;


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

    /* screen buffer */
    renderWidth = resolution;
    renderHeight = W_HEIGHT;
    int zbuffer[renderWidth * renderHeight];

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

    /*************
     * GAME LOOP *
     ************/

    while (running) {
        playerstate.sector_id = 0;
        lastTick = currentTick;
        currentTick = SDL_GetTicks();
        dt = (currentTick - lastTick) / 100.0f;
        cTick = SDL_GetTicks();

        SDL_GetWindowSizeInPixels(window, &window_width, &window_height);

        window_width_ratio = (float) window_width / W_WIDTH;
        window_height_ratio = (float) window_height / W_HEIGHT;

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
                playerstate.direction = normalize_direction(playerstate.direction - mousexmove);
                playerstate.zlook += mouseymove;
                ptr.x = event.motion.x;
                ptr.y = event.motion.y;
            }
        }


        /*********
         * LOGIC *
         *********/

        /* capping */
        if (playerstate.zlook > playerstate.max_zlook) {
            playerstate.zlook = playerstate.max_zlook;
        } 
        if (playerstate.zlook < -playerstate.max_zlook) {
            playerstate.zlook = -playerstate.max_zlook;
        }

        const bool *key_states = SDL_GetKeyboardState(NULL);

        int forward = (key_states[SDL_SCANCODE_W] - key_states[SDL_SCANCODE_S]);
        int sides   = (key_states[SDL_SCANCODE_A] - key_states[SDL_SCANCODE_D]);
        playerentity.hsp = lerp(playerentity.hsp, forward * cos(deg2rad(playerstate.direction)) - sides * cos(deg2rad(playerstate.direction+90)), 0.3);
        playerentity.vsp = lerp(playerentity.vsp, forward * sin(deg2rad(playerstate.direction)) - sides * sin(deg2rad(playerstate.direction+90)), 0.3);

        if (key_states[SDL_SCANCODE_E]) {
            playerstate.zlook = 0;
        }

        /* collisions */
        playerentity.x += playerentity.hsp;
        playerentity.y += playerentity.vsp;
        playerentity.z += (key_states[SDL_SCANCODE_SPACE] - key_states[SDL_SCANCODE_LSHIFT]);

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
        
        /* resetting the whole canvas */
        for (int i = 0; i < renderWidth * renderHeight; i ++) {
            pixels[i] = 0x0000;
        }
        for (int i = 0; i < renderWidth * renderHeight; i ++) {
            zbuffer[i] = 9999;
        }

        /* resetting */
        visible_sectors_index = 0;
        visible_sectors_ln = 0;

        lines_arr_index = 0;
        lines_arr_ln = 0;


        /* drawing maplines */

        double c = cos(deg2rad(-playerstate.direction));
        double s = sin(deg2rad(-playerstate.direction));

        sector current_sector = sectorslist[playerstate.sector_id];
        visible_sectors[visible_sectors_ln++] = current_sector;

        while (visible_sectors_index < visible_sectors_ln) {
            for (int i = 0; i < visible_sectors[visible_sectors_index].num_wall; i ++) { /* drawing all the walls */
                linedef line = visible_sectors[visible_sectors_index].lines[visible_sectors[visible_sectors_index].walls_id[i]];

                line.a.x -= playerentity.x;
                line.a.y -= playerentity.y;
                line.b.x -= playerentity.x;
                line.b.y -= playerentity.y;

                /* coords a */
                double ax = line.a.x;
                line.a.x = c * line.a.x - s * line.a.y;
                line.a.y = s * ax + c * line.a.y;

                /* coords b */
                double bx = line.b.x;
                line.b.x = c * line.b.x - s * line.b.y;
                line.b.y = s * bx + c * line.b.y;

                if (map) {
                SDL_FRect origin;
                origin.x = window_width/2-2;
                origin.y = window_height/2-2;
                origin.w = 4;
                origin.h = 4;
                SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
                if (line.isPortal) SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
                SDL_RenderLine(renderer, window_width/2 + line.a.y, window_height/2 - line.a.x, window_width/2 + line.b.y, window_height/2 - line.b.x);
                if (line.isPortal) SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
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
                }

                /* drawing the wall*/
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


                        float bottomDeltaA = ((float) (playerentity.z - line.z2) / tilesize) * heightA;
                        float bottomDeltaB = ((float) (playerentity.z - line.z2) / tilesize) * heightB;

                        float topDeltaA = ((float) (line.z1 - playerentity.z) / tilesize - 1.0) * heightA;
                        float topDeltaB = ((float) (line.z1 - playerentity.z) / tilesize - 1.0) * heightB;
                        
                        y11 = renderHeight/2 - heightA/2 - playerstate.zlook - topDeltaA;
                        y12 = renderHeight/2 + heightA/2 - playerstate.zlook + bottomDeltaA;

                        y21 = renderHeight/2 - heightB/2 - playerstate.zlook - topDeltaB;
                        y22 = renderHeight/2 + heightB/2 - playerstate.zlook + bottomDeltaB;

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

                        R_RenderWall(pixels, zbuffer, pnum, renderWidth, renderHeight, x1, x2, y11, y12, y21, y22, line, 1, 1, 1, 1);
                    }
                }
            }

            for (int i = 0; i < visible_sectors[visible_sectors_index].num_linedef - visible_sectors[visible_sectors_index].num_wall; i ++) { /* drawing all the portals */
                linedef line = visible_sectors[visible_sectors_index].lines[visible_sectors[visible_sectors_index].corresponding_portal_id[i]];

                line.a.x -= playerentity.x;
                line.a.y -= playerentity.y;
                line.b.x -= playerentity.x;
                line.b.y -= playerentity.y;

                /* coords a */
                double ax = line.a.x;
                line.a.x = c * line.a.x - s * line.a.y;
                line.a.y = s * ax + c * line.a.y;

                /* coords b */
                double bx = line.b.x;
                line.b.x = c * line.b.x - s * line.b.y;
                line.b.y = s * bx + c * line.b.y;

                if (map) {
                SDL_FRect origin;
                origin.x = window_width/2-2;
                origin.y = window_height/2-2;
                origin.w = 4;
                origin.h = 4;
                SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
                if (line.isPortal) SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
                SDL_RenderLine(renderer, window_width/2 + line.a.y, window_height/2 - line.a.x, window_width/2 + line.b.y, window_height/2 - line.b.x);
                if (line.isPortal) SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
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
                }

                /* drawing the wall*/
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
                        
                        double x1, x2, y11, y12, y21, y22, yportal11, yportal12, yportal21, yportal22;
                        x1 = pA.x;
                        x2 = pB.x;


                        float bottomDeltaA = ((float) (playerentity.z - line.z2) / tilesize) * heightA;
                        float bottomDeltaB = ((float) (playerentity.z - line.z2) / tilesize) * heightB;

                        float topDeltaA = ((float) (line.z1 - playerentity.z) / tilesize - 1.0) * heightA;
                        float topDeltaB = ((float) (line.z1 - playerentity.z) / tilesize - 1.0) * heightB;

                        float topPortalA = ((float) (line.portalZ1 - playerentity.z) / tilesize - 1.0) * heightA;
                        float topPortalB = ((float) (line.portalZ1 - playerentity.z) / tilesize - 1.0) * heightB;

                        float bottomPortalA = ((float) (playerentity.z - line.portalZ2) / tilesize) * heightA;
                        float bottomPortalB = ((float) (playerentity.z - line.portalZ2) / tilesize) * heightB;


                        y11 = renderHeight/2 - heightA/2 - playerstate.zlook - topDeltaA;
                        y12 = renderHeight/2 + heightA/2 - playerstate.zlook + bottomDeltaA;

                        yportal11 = renderHeight/2 - heightA/2 - playerstate.zlook - topPortalA;
                        yportal12 = renderHeight/2 + heightA/2 - playerstate.zlook + bottomPortalA;

                        yportal21 = renderHeight/2 - heightB/2 - playerstate.zlook - topPortalB;
                        yportal22 = renderHeight/2 + heightB/2 - playerstate.zlook + bottomPortalB;

                        y21 = renderHeight/2 - heightB/2 - playerstate.zlook - topDeltaB;
                        y22 = renderHeight/2 + heightB/2 - playerstate.zlook + bottomDeltaB;

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

                        R_RenderWall(pixels, zbuffer, pnum, renderWidth, renderHeight, x1, x2, y11, yportal11, y21, yportal21, line, 1, 1, 1, 1);
                        R_RenderWall(pixels, zbuffer, pnum, renderWidth, renderHeight, x1, x2, yportal12, y12, yportal22, y22, line, 1, 1, 1, 1);
                        portals[portals_ln++] = (wallsegment) {.portal_id = visible_sectors[visible_sectors_index].neighboring_sectors[i], .x1 = x1, .x2 = x2, .y11 = yportal11, .y12 = yportal12, .y21 = yportal21, .y22 = yportal22};
                    }
                }
            }

            /* load in more sectors */
            for (int p = 0; p < portals_ln; p ++) {
                if (R_WallSegmentVisible(zbuffer, pnum, renderWidth, renderHeight, portals[p])) {
                    printf("CONNECTED ID: %d. VISIBLE SECTORS: %d\n", portals[p].portal_id, visible_sectors_ln);
                    if (visible_sectors_ln < MAXVISSECTOR)
                        visible_sectors[visible_sectors_ln ++] = sectorslist[portals[p].portal_id];

                }
            }
            portals_ln = 0;
            portals_index = 0;
            visible_sectors_index ++;
        }
 
        if (map) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
            SDL_RenderLine(renderer, 0, window_height/2, window_width, window_height/2);
        }


        frames ++;
        uint64_t elapsedTick = SDL_GetTicks() - cTick;
        if (cTick > lTick + 1000) {
            lTick = cTick;
            snprintf(windowtitle, TITLELENGTH, "Raycaster FPS: %d, x: %d, y: %d", frames, (int) playerentity.x, (int) playerentity.y);
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
    SDL_UnlockTexture(screen_texture);
    SDL_DestroyTexture(screen_texture);
    SDL_Quit();

}