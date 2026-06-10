#include "hw.h"
#include "config.h"
#include "raycast.h"
#include "hud.h"
#include "floor.h"

#define TEX_W 16
#define TEX_H 16

/* 3x3 cross tile: 1=grout, 0=stone. pixel 1 if x%3==1 || y%3==1 */
static u8 floor_pattern[TEX_H * TEX_W] = {
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
static u8 lod1_pat[8 * 8] = {
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
static u16 lod0_col[2] = { 0x0B96, 0x0222 }; /* RGB(22,18,12) / RGB(4,4,4) */
/* LOD1: blend by grout count 0,2,3 out of 4 pixels */
static u16 lod1_col[4] = { 0x0B96, 0x0654, 0x6654, 0x4443 }; /* 0%, -, 50%, 75% grout */
/* LOD2: constant avg = 4/9*stone + 5/9*grout = RGB(12,10,8) */
static u16 lod2_col = 0x0654;

static u16 get_floor_texture_pixel(fix worldX, fix worldY, u16 c) {
    //int dist = (int)(rowDistance >> FBITS); /* integer world-unit distance */
    //if (dist >= 3) {
    //    return lod2_col;
    //} else if (dist >= 1) {
    //    int tx = (int)(worldX >> (FBITS-2)) & 7;
    //    int ty = (int)(worldY >> (FBITS-2)) & 7;
    //    return lod1_col[lod1_pat[ty * 8 + tx]];
    //} else {
    //    int tx = (int)(worldX >> (FBITS-2)) & (TEX_W - 1);
    //    int ty = (int)(worldY >> (FBITS-2)) & (TEX_H - 1);
    //    return lod0_col[floor_pattern[ty * TEX_W + tx]];
    //}
    //return (worldX >> FBITS) + (worldY >> FBITS);
    //u16 tx, ty;
    //__asm__ (
    //        "swap   %1  \n\t"
    //        "move.w %1, %0"
    //        : "=&d" (tx)
    //        : "d"   (worldX)
    //);
    //return tx;
    return floor_pattern[c % (TEX_H*TEX_W)]; 
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

void floor_render(void){
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
                
                u16 color = get_floor_texture_pixel(worldX, worldY, c);
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
