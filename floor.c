#include "hw.h"
#include "config.h"
#include "raycast.h"
#include "hud.h"
#include "floor.h"

#define TEX_W 16
#define TEX_H 16

/* 3x3 cross tile: 1=grout, 0=stone. pixel 1 if x%3==1 || y%3==1 */
static const u8 floor_pattern[TEX_H * TEX_W] = {
    /* y=0  */ 0,1,0, 0,1,0, 0,1,0, 0,1,0, 0,1,0, 0,
    /* y=1  */ 1,1,1, 1,1,1, 1,1,1, 1,1,1, 1,1,1, 1,
    /* y=2  */ 0,1,0, 0,1,0, 0,1,0, 0,1,0, 0,1,0, 0,
    /* y=3  */ 0,1,0, 0,1,0, 0,1,0, 0,1,0, 0,1,0, 0,
    /* y=4  */ 1,1,1, 1,1,1, 1,1,1, 1,1,1, 1,1,1, 1,
    /* y=5  */ 0,1,0, 0,1,0, 0,1,0, 0,1,0, 0,1,0, 0,
    /* y=6  */ 0,1,0, 0,1,0, 0,1,0, 0,1,0, 0,1,0, 0,
    /* y=7  */ 1,1,1, 1,1,1, 1,1,1, 1,1,1, 1,1,1, 1,
    /* y=8  */ 0,1,0, 0,1,0, 0,1,0, 0,1,0, 0,1,0, 0,
    /* y=9  */ 0,1,0, 0,1,0, 0,1,0, 0,1,0, 0,1,0, 0,
    /* y=10 */ 1,1,1, 1,1,1, 1,1,1, 1,1,1, 1,1,1, 1,
    /* y=11 */ 0,1,0, 0,1,0, 0,1,0, 0,1,0, 0,1,0, 0,
    /* y=12 */ 0,1,0, 0,1,0, 0,1,0, 0,1,0, 0,1,0, 0,
    /* y=13 */ 1,1,1, 1,1,1, 1,1,1, 1,1,1, 1,1,1, 1,
    /* y=14 */ 0,1,0, 0,1,0, 0,1,0, 0,1,0, 0,1,0, 0,
    /* y=15 */ 0,1,0, 0,1,0, 0,1,0, 0,1,0, 0,1,0, 0,
};

static u16 scb2buf_floor[NUM_COLS];
static u16 scb3buf_floor[NUM_COLS];

/* LOD1: 2×2 box-filter of floor_pattern → 8×8, values = grout count in 2×2 (0,2,3) */
static const u8 lod1_pat[8 * 8] = {
    /* y=0 */ 3,2,3,3,2,3,3,2,
    /* y=1 */ 2,0,2,2,0,2,2,0,
    /* y=2 */ 3,2,3,3,2,3,3,2,
    /* y=3 */ 3,2,3,3,2,3,3,2,
    /* y=4 */ 2,0,2,2,0,2,2,0,
    /* y=5 */ 3,2,3,3,2,3,3,2,
    /* y=6 */ 3,2,3,3,2,3,3,2,
    /* y=7 */ 2,0,2,2,0,2,2,0,
};

/* LOD0: exact stone/grout */
static const u16 lod0_col[2] = { 0x0B96, 0x0222 }; /* RGB(22,18,12) / RGB(4,4,4) */
/* LOD1: blend by grout count 0,2,3 out of 4 pixels */
static const u16 lod1_col[4] = { 0x0B96, 0x0654, 0x6654, 0x4443 }; /* 0%, -, 50%, 75% grout */
/* LOD2: constant avg = 4/9*stone + 5/9*grout = RGB(12,10,8) */
static const u16 lod2_col = 0x0654;

static u16 get_floor_texture_pixel(fix worldX, fix worldY, fix rowDistance) {
    int dist = (int)(rowDistance >> FBITS); /* integer world-unit distance */
    //return (((worldX >> FBITS) % 2) ^ ((worldY >> FBITS) % 2)) ? (lod2_col):(0); 

    if (dist >= 5) {
        return lod2_col;
    } else if (dist >= 1) {
        int tx = (int)(worldX >> (FBITS-2)) & 7;
        int ty = (int)(worldY >> (FBITS-2)) & 7;
        return lod1_col[lod1_pat[ty * 8 + tx]];
    } else {
        int tx = (int)(worldX >> (FBITS-2)) & (TEX_W - 1);
        int ty = (int)(worldY >> (FBITS-2)) & (TEX_H - 1);
        return lod0_col[floor_pattern[ty * TEX_W + tx]];
    }
}

void init_floor(void){
    for(u16 c = 0; c < NUM_COLS; c++){
        u16 spr = FLOOR_BASE + c;

        /* SCB1: 15 tiles, with difrent Tile-Index, same palat */
        vram_addr(VRAM_SCB1 + spr * 64);
        vram_mod(1);
        for(u16 band = 0; band < 15;band++){
            vram_w((u16)(TILE_FLOOR_BASE + band));  /* Tile 3..17 */
            vram_w((u16)((PAL_FLOOR_BASE + c) << 8));    /* palat */
        }

        scb4(spr, (u16)(c * COLW)); /* X-Position fixed */
        scb2(spr, HSHRINK, 0);      /* width fixed, height 0 */
        scb3(spr, SCRH + 32, 0, 15);/* hide it */
    }
}

#define SPR_PX_COUNT 15
void floor_render(void){
    fix posX, posY, dirX, dirY, planeX, planeY;
    rc_camera(&posX, &posY, &dirX, &dirY, &planeX, &planeY);

    /* calculate the 15 world cords. and the x_step */
    fix world_cords[2][SPR_PX_COUNT] = {0};
    
    // compile-time constants !!!
    static const fix cameraX_0 = 0 - FONE; //for column 0
    static const fix cameraX_1 = (fix)((FONE << 1) / (NUM_COLS - 1)) - FONE;

    /* floor takes half of the screen */
    static const u16 floor_top = SCRH / 2;
    static const u16 floor_bot = SCRH;
    static const u16 range = (floor_bot - floor_top);

    fix x_step[SPR_PX_COUNT], y_step[SPR_PX_COUNT];
    fix rowDist[SPR_PX_COUNT];

    /* world cord, disance and step calculation*/
    for(int c = 0; c < SPR_PX_COUNT;c++){
        u16 y = floor_top + (u16)(((2*c + 1) * range) / (SPR_PX_COUNT * 2)); /* could be pre calculateted */
        u16 denom = (y > SCRH/2) ? (u16)(y - SCRH/2) : 1;                    // this too
        fix rowDistance = (fix)(((s32)(SCRH/2) << FBITS) / denom);           // this too
        rowDist[c] = (fix)((SCRH / 2) * FONE) / (y - SCRH / 2);              // this too

        /* Thats the only thing that realy has to be calculatet live */
        world_cords[0][c] = posX + fmul(rowDistance, dirX + fmul(planeX, cameraX_0));
        world_cords[1][c] = posY + fmul(rowDistance, dirY + fmul(planeY, cameraX_0));

        x_step[c] = (posX + fmul(rowDist[c], dirX + fmul(planeX, cameraX_1))) - world_cords[0][c];
        y_step[c] = (posY + fmul(rowDist[c], dirY + fmul(planeY, cameraX_1))) - world_cords[1][c];
    }


    /* getting the floor pixel information */
    for(u16 c=0; c < NUM_COLS; c++){
        u16 floor_slice_top = floor_clip[0][c];
        u16 floor_slice_bot = floor_clip[1][c];
        u16 range_slice = (floor_slice_bot - floor_slice_top);

        u16 i_start = (floor_slice_top - floor_top) * SPR_PX_COUNT / range;
        u16 i_end   = (floor_slice_bot - floor_top) * SPR_PX_COUNT / range;
        
        u16 r_start = floor_top + (u16)((u32)i_start * range / SPR_PX_COUNT);
        u16 r_range = (u16)((u32)(i_end - i_start) * range / SPR_PX_COUNT);


        /* sprite position and length */
        u16 tile_count = i_end - i_start;
        u16 vsh_f = tile_count > 0 ? (u16)(((u32)r_range * 16u - 1u) / tile_count) : 0u;
        scb2buf_floor[c] = (u16)((HSHRINK << 8) | vsh_f);
        scb3buf_floor[c] = scb3_word(r_start, 0, tile_count);  /* works only if it goes completely down !*/

        //color palette berechnen
        int j = 0;
        for(u16 i = 0; i < SPR_PX_COUNT;i++){
            fix x = world_cords[0][i] + x_step[i];
            fix y = world_cords[1][i] + y_step[i];

            if(i >= i_start && i <= i_end){
                u16 color = get_floor_texture_pixel(x, y, rowDist[i]);
                pal_set(PAL_FLOOR_BASE + c, (u16)(j + 1), color);
                j++;
            }
            world_cords[0][i] = x;
            world_cords[1][i] = y;
        }
        
    }


}

void floor_render_old(void){
        /* for each wall slice (NUM_COLS):
         *      take the clipping information y1 and y2
         *      devide it to get the pixel positions (/15)
         *      for all pixel positions:
         *          Floor Casting (x, y):
         *              rowDistance = (SCRH/2) / (y - SCRH/2)
         *              cameraX = 2*x/SCRW - 1
         *              worldX = posX + rowDistance * (dirX + planeX * cameraX)
         *              worldY = posY + rowDistance * (dirY + planeY * cameraX)
         *          pixel = get_floor_texture_pixel(worldX, worldY)
         *      put 15pixel information in the color palat for this floor slice. (bufferd, it needs to be writen to while vblank ! )
         *      give the floor slices the correct y startpoint and height (bufferd)
         * */
        fix posX, posY, dirX, dirY, planeX, planeY;
        rc_camera(&posX, &posY, &dirX, &dirY, &planeX, &planeY);

        for(u16 c = 0; c < NUM_COLS;c++){
            u16 floor_top = floor_clip[0][c];
            u16 floor_bot = floor_clip[1][c];

            u16 range = (floor_bot - floor_top);
            
            //if(range == 0) continue;
            fix cameraX = (fix)(((int64_t)2 * FONE * c) / (NUM_COLS - 1)) - FONE;
            for(u16 i = 0; i < 15; i++){
                u16 y = floor_top + (u16)(((2*i + 1) * range) / 30); /* i+0.5*/
                u16 denom = (y > SCRH/2) ? (u16)(y - SCRH/2) : 1;
                fix rowDistance = (fix)(((s32)(SCRH/2) << FBITS) / denom);
                fix worldX = posX + fmul(rowDistance, dirX + fmul(planeX, cameraX));
                fix worldY = posY + fmul(rowDistance, dirY + fmul(planeY, cameraX));
                
                u16 color = get_floor_texture_pixel(worldX, worldY, rowDistance);
                pal_set(PAL_FLOOR_BASE + c, (u16)(i + 1), color);
            }

            /* (vshrink+1)*240/256 px displayed; invert: vshrink=(range*16-1)/15 */
            u16 vsh_f = range > 0 ? (u16)(((u32)range * 16u - 1u) / 15u) : 0u;
            scb2buf_floor[c] = (u16)((HSHRINK << 8) | vsh_f);
            scb3buf_floor[c] = scb3_word(floor_top, 0, 15);
        }
}

void floor_blit(void) {
    vram_addr(VRAM_SCB2 + FLOOR_BASE);
    vram_mod(1);
    for(u16 c = 0; c < NUM_COLS; c++) vram_w(scb2buf_floor[c]);

    vram_addr(VRAM_SCB3 + FLOOR_BASE);
    vram_mod(1);
    for(u16 c = 0; c < NUM_COLS; c++) vram_w(scb3buf_floor[c]);
}
