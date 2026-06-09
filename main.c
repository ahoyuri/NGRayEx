/* main.c - boot setup and the game loop.
 
 */
#include "hw.h"
#include "config.h"
#include "raycast.h"
#include "hud.h"
#include "floor.h"

/* ---- palette setup --------------------------------------------------- */
static void init_palettes(void) {
    /* index 0 of every palette is transparent for sprites; we keep walls
     * opaque by only using indices 1..3. */

    /* wall, lit (N/S faces): mortar / brick / highlight */
    pal_set(PAL_WALL_LIT, 1, RGB( 9,  9, 10));
    pal_set(PAL_WALL_LIT, 2, RGB(24,  8,  6));
    pal_set(PAL_WALL_LIT, 3, RGB(29, 14, 12));

    /* wall, dark (E/W faces): same hues, dimmer */
    pal_set(PAL_WALL_DARK, 1, RGB( 5,  5,  6));
    pal_set(PAL_WALL_DARK, 2, RGB(14,  4,  3));
    pal_set(PAL_WALL_DARK, 3, RGB(18,  8,  7));

    /* distance-shaded wall palettes Shading toward black. */
    {
        /* index:        1=body 2=body 3=highlight  (1 and 2 share the wall colour) */
        static const u8 base_r[3] = {24, 24, 29};
        static const u8 base_g[3] = { 8,  8, 14};
        static const u8 base_b[3] = { 6,  6, 12};
        for (int b = 0; b < DEPTH_BANDS; b++) {
            int fn = 256 - (b * 200) / (DEPTH_BANDS - 1);   /* 256 near .. 56 far */
            for (int s = 0; s < 2; s++) {
                int sf = s ? 140 : 256;                     /* dark side ~55%     */
                u16 pal = PAL_DEPTH_BASE + s * DEPTH_BANDS + b;
                for (int i = 0; i < 3; i++) {
                    int r = base_r[i] * fn / 256 * sf / 256;
                    int g = base_g[i] * fn / 256 * sf / 256;
                    int bl = base_b[i] * fn / 256 * sf / 256;
                    pal_set(pal, (u16)(i + 1), RGB((u8)r, (u8)g, (u8)bl));
                }
            }
        }
    }

    /* backdrop bands */
    pal_set(PAL_CEILING, 1, RGB( 6,  7, 12));    /* night sky / ceiling     */
    pal_set(PAL_FLOOR,   1, RGB(12, 10,  7));    /* dusty floor             */

    /* minimap */
    pal_set(PAL_MAP_WALL,   15, RGB(20, 20, 22)); /* walls */
    pal_set(PAL_MAP_PLAYER, 15, RGB( 4, 31,  8)); /* player */

    REG_BACKDROP = RGB(0, 0, 0);
}

/* ---- clear the fix layer */
static void clear_fix(void) {
    vram_addr(VRAM_FIX);
    vram_mod(1);
    for (int i = 0; i < 40 * 32; i++) vram_w(0x0000);
}


 
static void disable_sprites(void) {
    for (u16 s = 1; s < SPR_TOTAL; s++) {
        scb2(s, 0x0F, 0x00);          /* full width, zero height            */
        scb3(s, SCRH + 32, 0, 1);     /* below the visible area             */
        scb4(s, 0);
    }
}

/* ---- static floor/ceiling backdrop: BG_COUNT full-width columns ------  */
static void init_background(void) {
    for (u16 i = 0; i < BG_COUNT; i++) {
        u16 spr = BG_BASE + i;
        for (u16 t = 0; t < BG_WIN; t++) {
            u16 pal = (t < BG_SPLIT) ? PAL_CEILING : PAL_FLOOR;
            scb1_tile(spr, t, TILE_SOLID, pal);
        }
        scb2(spr, 0x0F, 0xFF);        /* full size, no shrink (16-tile ref)  */
        scb3(spr, 0, 0, BG_WIN);      /* top of screen                       */
        scb4(spr, i * 16);
    }
}

/* ---- wall-slice sprites: fixed X + brick tilemap set once; SCB2/SCB3
 * (and palette when shading flips) are updated every frame in rc_blit. --- */
static void init_walls(void) {
    for (u16 c = 0; c < NUM_COLS; c++) {
        u16 spr = WALL_BASE + c;
        scb1_fill(spr, WALL_WIN, TILE_BRICK, PAL_WALL_LIT);
        scb4(spr, c * COLW);
        scb2(spr, HSHRINK, 0x00);     
        scb3(spr, 0, 0, WALL_WIN);
    }
}


int main(void) {
    watchdog_kick();
    clear_fix();
    init_palettes();
    disable_sprites();
    /* init_background(); */ /* backdrop diabled for floor test*/
    init_walls();
    hud_init();
    rc_init();
    init_floor();
    hud_draw_minimap();

    for (;;) {
        u8 pressed = (u8)~REG_P1CNT;    
        rc_input(pressed);

        u16 line_before = REG_LSPCMODE >> 7;
        u16 vbl_before = vblank_count;
        rc_render();                    /* DDA during active display          */
        floor_render();
        u16 frames = vblank_count - vbl_before; /* completed Frames */
        u16 line_after = REG_LSPCMODE >> 7;
        s16 sub_lines = (s16)(line_after - line_before);
        
        u16 total_lines = (u16)(frames * 264 + sub_lines);
        int fps = total_lines > 0 ? (264 * 60)/total_lines : 999;

        wait_vblank();
        watchdog_kick();
        rc_blit();                      /* push to VRAM during vblank         */
        floor_blit();

        /* button C toggles the minimap  */
        {
            enum { C = 0x40 };          /* P1 bit 6                            */
            static u8 c_prev = 0;
            u8 c_now = pressed & C;
            if (c_now && !c_prev) {
                hud_map_on = !hud_map_on;
                if (hud_map_on) { hud_draw_minimap(); }
                else              hud_clear_minimap();
            }
            c_prev = c_now;
        }

        hud_update_marker();            /* 2 fix writes when the cell changes */
        
        
        hud_draw_fps(fps);
        hud_draw_time(total_lines);
    }
    return 0;
}
