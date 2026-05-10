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
#include "p_playervariables.c"

#define SHADE 150.0


int worldmap[LEVEL_HEIGHT][LEVEL_WIDTH] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1,},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1,},
    {1, 0, 1, 0, 0, 1, 1, 1, 1, 1,},
    {1, 0, 0, 1, 0, 0, 0, 0, 0, 1,},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1,},
};

float zmap[LEVEL_HEIGHT][LEVEL_WIDTH] = {
    {5., 5., 5., 5., 5., 5., 5., 5., 5., 5.,},
    {5., 0., 0., 0., 0., 0., 0., 0., 0., 5.,},
    {5., 0., 3., 0., 0., .2, .3, 5.2, .5, 5.,},
    {5., 0., 0.,.35, 0., 0., 0., 0., 0., 5.,},
    {5., 5., 5., 5., 5., 5., 5., 5., 5., 5.,},
};

bool rect_collide(SDL_FRect *a, SDL_FRect *b) {
    if (a->x + a->w >= b->x && a->x <= b->x + b->w && a->y + a->h >= b->y && a->y <= b->y + b->h) {
        return true;
    }
    return false;
}

int main(void) {

    /* window */
    char *windowtitle = (char*) malloc(sizeof(char) * 30);

    snprintf(windowtitle, 30, "FPS: %d", 0);

    /* game variables */
    bool running = true;

    /* camera */
    P_PlayerState playerstate = P_InitializePlayerState();
    E_GameEntity playerentity = {.x = 144, .y = 112, .z = 64, .w = 16, .h = 16};

    int resolution = 320;

    float dt;
    uint64_t cTick = 0;
    uint64_t lTick = 0;
    int frames = 0;

    uint32_t currentTick = 0;
    uint32_t lastTick = 0;

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

    /* texture buffer */
    u_int32_t* texpixels;
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

    SDL_SetDefaultTextureScaleMode(renderer, SDL_SCALEMODE_NEAREST);

    texpitch = G_LoadTextureIntoBitmapForm("images/brick_texture.png", &texpixels);

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
        pnum = pitch / 4;
        printf("Init Screen:\n");
        printf("\tSize of pixel buffer: %lu\n", sizeof(pixels));
        printf("\tPitch value of texture buffer: %d\n", pitch);
    } else {
        SDL_Log("Failed to lock screen texture! Error: %s", SDL_GetError());
        SDL_Quit();
        return SDL_APP_FAILURE;
    }

    /* running */
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);

    while (running) {

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
                } else if (event.key.key == SDLK_DOWN) {
                    playerstate.fov_mult -= 0.1;
                    printf("Plane increase: %f\n", playerstate.fov_mult);
                } else if (event.key.key == SDLK_UP) {
                    playerstate.fov_mult += 0.1;
                    printf("Plane increase: %f\n", playerstate.fov_mult);
                }
            } else if (event.type == SDL_EVENT_MOUSE_MOTION) {
                double mousexmove = -event.motion.xrel;
                double mouseymove = event.motion.yrel;
                double oldDirX = playerstate.dirX;
                playerstate.dirX = playerstate.dirX * cos(deg2rad(-mousexmove * dt)) - playerstate.dirY * sin(deg2rad(-mousexmove * dt));
                playerstate.dirY = oldDirX * sin(deg2rad(-mousexmove * dt)) + playerstate.dirY * cos(deg2rad(-mousexmove * dt));
                playerstate.zlook += mouseymove;
            }
        }

        /* logic */

        /* capping */
        if (playerstate.zlook > playerstate.max_zlook) {
            playerstate.zlook = playerstate.max_zlook;
        }
        if (playerstate.zlook < -playerstate.max_zlook) {
            playerstate.zlook = -playerstate.max_zlook;
        }
        
        playerstate.fov_mult = SDL_max(SDL_min(playerstate.fov_mult, 2), 0.1);

        const bool *key_states = SDL_GetKeyboardState(NULL);

        int forward = (key_states[SDL_SCANCODE_W] - key_states[SDL_SCANCODE_S]);
        int sides   = (key_states[SDL_SCANCODE_A] - key_states[SDL_SCANCODE_D]);
        playerentity.hsp = lerp(playerentity.hsp, forward * playerstate.dirX + sides * playerstate.planeX, 0.3);
        playerentity.vsp = lerp(playerentity.vsp, forward * playerstate.dirY + sides * playerstate.planeY, 0.3);

        if (key_states[SDL_SCANCODE_E]) {
            // playerstate.zlook = 0;
            playerstate.fov_mult = 1.0;
            playerentity.z = lerp(playerentity.z, 32, 0.2);
        } else {
            // playerentity.z = lerp(playerentity.z, 0, 0.2);
            int mapPosX = (int) (playerentity.x / tilesize);
            int mapPosY = (int) (playerentity.y / tilesize);
            playerentity.z = lerp(playerentity.z, zmap[mapPosY][mapPosX] * tilesize, 0.2);
        }

        /* collisions */
        SDL_FRect surroundingHBs[9];
        int count = 0;
        float topleftX = playerentity.x - playerentity.w/2;
        float topleftY = playerentity.y - playerentity.h/2;
        SDL_FRect hitbox;
        hitbox.x = topleftX;
        hitbox.y = topleftY;
        hitbox.w = playerentity.w;
        hitbox.h = playerentity.h;

        int cellX = (int) (playerentity.x / tilesize);
        int cellY = (int) (playerentity.y / tilesize);

        for (int y = -1; y < 2; y ++) {
            for (int x = -1; x < 2; x ++) {
                if (worldmap[cellY+y][cellX+x] != 0) {
                    SDL_FRect newHB;
                    newHB.x = (cellX+x) * tilesize;
                    newHB.y = (cellY+y) * tilesize;
                    newHB.w = newHB.h = tilesize;
                    surroundingHBs[count++] = newHB;
                }
            }
        }
        // for (int c = 0; c < count; c ++) {
        //     SDL_FRect hb = surroundingHBs[c];
        //     hitbox.x = topleftX + playerentity.hsp;
        //     if (rect_collide(&hitbox, &hb)) {
        //         playerentity.hsp = 0;
        //     }
        //     hitbox.x = topleftX;
        //     hitbox.y = topleftY + playerentity.vsp;
        //     if (rect_collide(&hitbox, &hb)) {
        //         playerentity.vsp = 0;
        //     }
        //     hitbox.y = topleftY;
        // }


        playerentity.x += playerentity.hsp;
        playerentity.y += playerentity.vsp;

        /* plane maths */
        playerstate.planeX = playerstate.dirY * playerstate.fovratio * playerstate.fov_mult;
        playerstate.planeY = -playerstate.dirX * playerstate.fovratio * playerstate.fov_mult;

        /* rendering */
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

        double posX = playerentity.x / tilesize;
        double posY = playerentity.y / tilesize;

        int tex_width = 16;
        int tex_height = 16;

        /* floorcast */
        // for (int y = (int) renderHeight/2 - playerstate.zlook; y < renderHeight; y ++) {
        //     float rayDirX0 = playerstate.dirX + playerstate.planeX;
        //     float rayDirY0 = playerstate.dirY + playerstate.planeY;
        //     float rayDirX1 = playerstate.dirX - playerstate.planeX;
        //     float rayDirY1 = playerstate.dirY - playerstate.planeY;

        //     float p = y - renderHeight/2 + playerstate.zlook;
        //     float posZ = 0.35 * renderHeight;
        //     float rowDistance = (posZ / p) / tan(playerstate.initfov / 2) / playerstate.fov_mult;

        //     float floorStepX = rowDistance * (rayDirX1 - rayDirX0) / (float) renderWidth;
        //     float floorStepY = rowDistance * (rayDirY1 - rayDirY0) / (float) renderWidth;

        //     float floorX = posX + rowDistance * rayDirX0;
        //     float floorY = posY + rowDistance * rayDirY0;


        //     for (int x = 0; x < resolution; x ++) {
        //         if (y > 0 && y < renderHeight && x > 0 && x < renderWidth) {
        //             if (!filled[y * pnum + x]) {
        //                 int cellX = (int) floorX;
        //                 int cellY = (int) floorY;

        //                 /* elevated */
        //                 int tX = (int) (tex_width * (floorX - cellX)) & (tex_width - 1);
        //                 int tY = (int) (tex_height * (floorY - cellY)) & (tex_height - 1);

        //                 floorX += floorStepX;
        //                 floorY += floorStepY;

        //                 cellX = SDL_clamp(cellX, 0, 10);
        //                 cellY = SDL_clamp(cellY, 0, 5);

        //                 u_int32_t color = texpixels[tY * texpnum + tX];
        //                 u_int32_t r, g, b, a;
        //                 G_GetARGBColor(color, &r, &g, &b, &a);
        //                 u_int32_t shade = G_MakeARGB32BitFormat(r - SHADE, g - SHADE, b - SHADE, a);
        //                 pixels[y * pnum + x] = (cellX == 1 && cellY == 1) ? shade : color;
        //             }
        //         }
        //     }
        // }

        /*** 
         * @concept: Wall obscuration PSEUDOCODE:
         * yMinimum = renderHeight;
         * for i = 0 to wallcount - 1
         *      if yColumn < yMinimum:
         *          yMinimum = yColumn
         *          render wall from yMinimum
         *          set filled to true.
         *      else:
         *          break
         * @concept: allows correct rendering of stairs.
        */

        /* raycast */
        for (int i = 0; i < resolution; i ++) {

            /// @concept: MAYBE THE HEIGHT FOR WHICH THE WALL SHOULD BE OBSCURED NEED TO BE THE HIGHEST HEIGHT WE SAW IN A STRIP GOING OUT.
            /// @protocol: Casting ray, then find the highest height seen. When the wall of that height is reached, then start to obscure everything else.
            /// @concept: 
            
            int MAXWALL = 20;
            int wallbuff[MAXWALL];
            float heightbuff[MAXWALL];
            int texbuff[MAXWALL];
            int sidebuff[MAXWALL];
            int geometrybuff[MAXWALL];
            int walltop[MAXWALL];
            int wallcount = 0;
            float previous_height = 0;

            int mapPosX = (int) posX;
            int mapPosY = (int) posY;
            double cameraX = 2.0 * i / (double) resolution - 1.0;

            double rayDirX = playerstate.dirX + playerstate.planeX * cameraX;
            double rayDirY = playerstate.dirY + playerstate.planeY * cameraX;

            double deltaDistX = (rayDirX == 0) ? 1e30 : fabs(1/rayDirX);
            double deltaDistY = (rayDirY == 0) ? 1e30 : fabs(1/rayDirY);
            double perpWallDist;
            double sideDistX;
            double sideDistY;

            int stepX;
            int stepY;

            int hit = 0;
            int side;

            if (rayDirX < 0) {
                stepX = -1;
                sideDistX = (double) (posX - mapPosX) * deltaDistX;
            } else {
                stepX = 1;
                sideDistX = (double) (mapPosX + 1.0 - posX) * deltaDistX;
            }

            if (rayDirY < 0) {
                stepY = -1;
                sideDistY = (double) (posY - mapPosY) * deltaDistY;
            } else {
                stepY = 1;
                sideDistY = (double) (mapPosY + 1.0 - posY) * deltaDistY;
            }


            side = (sideDistX >= sideDistY);

            if (side == 0)
                perpWallDist = fabs(sideDistX);
            else
                perpWallDist = fabs(sideDistY);

            int height = SDL_min(RAYCAST_SIZE_SCALE / (perpWallDist * 2), 1280);

            /* if current block is a wall */
            if (worldmap[mapPosY][mapPosX] != 0) {
                wallbuff[wallcount] = height;
                texbuff[wallcount] = 0;
                sidebuff[wallcount] = side;
                heightbuff[wallcount] = zmap[mapPosY][mapPosX];
                geometrybuff[wallcount] = 0;
                walltop[wallcount] = 1;
                wallcount ++;
            }
            
            /* NEW WALL BUFFER SYSTEM */

            int hasHitWall = 0;

            while (!hit) {

                /* tracing */
                if (sideDistX < sideDistY) {
                    sideDistX += deltaDistX;
                    mapPosX += stepX;
                    side = 0;
                } else {
                    sideDistY += deltaDistY;
                    mapPosY += stepY;
                    side = 1;
                }

                /* where walls start*/
                if (worldmap[mapPosY][mapPosX] != 0) {
                    hasHitWall = 1;
                }

                /* only started scanning for walls */
                if (hasHitWall) {
                    if (wallcount < MAXWALL) {
                        int canview = 1;

                        if (side == 0)
                            perpWallDist = fabs(sideDistX - deltaDistX);
                        else
                            perpWallDist = fabs(sideDistY - deltaDistY);
                        
                        float wallX = 0.0;
                        if (side == 0)
                            wallX = posY + perpWallDist * rayDirY;
                        else
                            wallX = posX + perpWallDist * rayDirX;
                        wallX -= (int) wallX;

                        // Texture strip
                        int texX = wallX * tex_width;
                        if (side == 0 && rayDirX > 0) texX = tex_width - texX - 1;
                        if (side == 1 && rayDirY > 0) texX = tex_width - texX - 1;

                        int height = SDL_min(RAYCAST_SIZE_SCALE / (perpWallDist * 2), 1280);
                        
                        if (wallcount > 0) { // For each wall, if there is a previous wall that is occupied by a cell
                            previous_height = heightbuff[wallcount-1];
                            if (wallcount < MAXWALL && geometrybuff[wallcount-1]) { // && geometrybuff[wallcount-1]) {
                                wallbuff[wallcount] = height;
                                heightbuff[wallcount] = previous_height;
                                texbuff[wallcount] = texX;
                                sidebuff[wallcount] = side;
                                geometrybuff[wallcount] = worldmap[mapPosY][mapPosX];

                                if (geometrybuff[wallcount-1] != 0) {/* if previously there was a wall, then only the top rim of the wall should be drawn*/
                                    walltop[wallcount] = 1;
                                }
                                wallcount ++;
                            }
                        }

                        if (wallcount < MAXWALL) {
                            wallbuff[wallcount] = height;
                            texbuff[wallcount] = texX;
                            sidebuff[wallcount] = side;
                            heightbuff[wallcount] = zmap[mapPosY][mapPosX];
                            geometrybuff[wallcount] = worldmap[mapPosY][mapPosX];
                            walltop[wallcount] = 0;
                            wallcount ++;
                        }

                    } else {break;}
                }

                /* where walls end */
                if (zmap[mapPosY][mapPosX] >= 5) {
                    hit = 1;
                }
            }

            /* drawing on screen */

            /* negative is looking up, positive is looking down */            
            float prevWallHeight = 0;
            for (int w = 0; w < wallcount; w ++) {
                int height = wallbuff[w] / playerstate.fov_mult;
                int texX = texbuff[w];
                int side_ = sidebuff[w];
                float wallheight = SDL_min(heightbuff[w], maxheight);

                int z_offset = 0;
                float frac = wallheight - (int) wallheight;
                int mult = (int) wallheight;

                int x = resolution - i - 1;

                if (w > 0) {
                    prevWallHeight = heightbuff[w-1];
                }
                
                /* floor rendering */
                int wallTopLimit = (w > 0) ? ((heightbuff[w-1] > heightbuff[w]) ? 0 : walltop[w]) : walltop[w];

                for (int y = (int) renderHeight/2 + height/2 - playerstate.zlook - prevWallHeight * height - wallTopLimit + (playerentity.z / 32.0f) * height; y < renderHeight; y ++) {
                    if (y > 0) {
                        if (filled[y * renderWidth + x]) {
                            break;
                        }
                        if (wallheight > 0.1) {
                            pixels[y * renderWidth + x] = 0xffaaaaaa;
                            filled[y * renderWidth + x] = true;
                        }
                    }
                }

                /* mult part */
                while (mult > 0) {
                    if (renderHeight/2 + height/2 - z_offset * height - playerstate.zlook + ((float) playerentity.z / 32.0f) * height > 0) {
                        for (int y = (int) renderHeight/2 - height/2 - z_offset * height + ((float) playerentity.z / 32.0f) * height; y < renderHeight/2 + height/2 - z_offset * height + ((float) playerentity.z / 32.0f) * height && y < renderHeight/2 + height/2 + ((float) playerentity.z / 32.0f) * height; y ++) {
                            if (y - (int) playerstate.zlook >= 0 && y - (int) playerstate.zlook < renderHeight && x >= 0 && x < renderWidth) {
                                if (filled[(y - (int) playerstate.zlook) * renderWidth + x]) {
                                    break;
                                }
                                int tx, ty;
                                tx = texX;
                                ty = ((float) (y - (renderHeight/2 - height/2 - z_offset * height + ((float) playerentity.z / 32.0f) * height)) / height) * (tex_width-1);
                                if (tx < 0 || tx > 15 || ty < 0 || ty > 15)
                                    printf("%d, %d\n", tx, ty);
                                u_int32_t color = texpixels[ty * texpnum + tx];
                                if (color != 0) {
                                    u_int32_t r, g, b, a;
                                    G_GetARGBColor(color, &r, &g, &b, &a);
                                    u_int32_t darker = G_MakeARGB32BitFormat(r - SHADE, g - SHADE, b - SHADE, a);
                                    pixels[(y - (int) playerstate.zlook) * pnum + x] = (side_ == 0) ? color : darker;
                                    filled[(y - (int) playerstate.zlook) * pnum + x] = true;
                                }
                            }
                        }
                        // wallTopLimit = 0;
                    } else {
                        break;
                    }
                    z_offset++;
                    mult --;
                }

                /* frac part */
                int fracHeight = frac * height;
                if (fracHeight > 0.01) {
                    int step = 0;
                    int y;
                    if ((w > 0) ? (heightbuff[w] >= heightbuff[w-1]) : true)
                    for (y = (int) renderHeight/2 - height/2 - z_offset * height + (height-fracHeight) + (playerentity.z / 32.0f) * height; y < renderHeight/2 + height/2 - z_offset * height + (playerentity.z / 32.0f) * height && y < renderHeight/2 + height/2 + (playerentity.z / 32.0f) * height; y ++) {
                        if (y - (int) playerstate.zlook >= 0 && y - (int) playerstate.zlook < renderHeight && x >= 0 && x < renderWidth) {
                            if (filled[(y - (int) playerstate.zlook) * renderWidth + x]) {
                                break;
                            }
                            /* texture data */
                            int tx, ty;
                            tx = texX;
                            ty = ((float) (y - (renderHeight/2 - height/2 - z_offset * height + (height-fracHeight) +  (playerentity.z / 32.0f) * height)) / height) * tex_height;
                            u_int32_t color = texpixels[ty * texpnum + tx];
                            if (color != 0) {
                                u_int32_t r, g, b, a;
                                G_GetARGBColor(color, &r, &g, &b, &a);
                                u_int32_t darker = G_MakeARGB32BitFormat(r - SHADE, g - SHADE, b - SHADE, a);
                                pixels[(y - (int) playerstate.zlook) * pnum + x] = (step == 0 ? 0xff00ffff : (!side_) ? color : darker); //(!wallTopLimit) ? color : 0xff00ffff; //(!side_) ? color : darker;
                                filled[(y - (int) playerstate.zlook) * pnum + x] = true;
                            }
                            if (wallTopLimit) {
                                break;
                            }
                            step ++;
                        }
                    }
                }
            }
        }

        frames ++;
        uint64_t elapsedTick = SDL_GetTicks() - cTick;
        if (cTick > lTick + 1000) {
            lTick = cTick;
            snprintf(windowtitle, 30, "Raycaster FPS: %d", frames);
            SDL_SetWindowTitle(window, windowtitle);
            frames = 0;
        }
        SDL_UnlockTexture(screen_texture);
        SDL_RenderTexture(renderer, screen_texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }


    SDL_Log("Renderer name: %s\n", SDL_GetRendererName(renderer));

    /* freeing everything */
    free(texpixels);
    free(windowtitle);
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_UnlockTexture(screen_texture);
    SDL_DestroyTexture(screen_texture);
    SDL_Quit();

}