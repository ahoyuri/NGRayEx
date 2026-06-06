#include "hud.h"
#include "config.h"
#include "map.h"
#include "raycast.h"

u8 hud_map_on = 1;
volatile u16 vblank_count = 0; 

static int prev_px = -1, prev_py = -1;

static void map_cell(int mx, int my, u16 pal, u16 tile) {
    fix_poke((u16)(MAP_FIX_COL + mx), (u16)(MAP_FIX_ROW + my), pal, tile);
}

void hud_draw_minimap(void) {
    for (int my = 0; my < MAP_H; my++)
        for (int mx = 0; mx < MAP_W; mx++) {
            if (map_at(mx, my))
                map_cell(mx, my, PAL_MAP_WALL, FIX_SOLID);
            else
                map_cell(mx, my, 0, FIX_BLANK);
        }
}

void hud_clear_minimap(void) {
    for (int my = 0; my < MAP_H; my++)
        for (int mx = 0; mx < MAP_W; mx++)
            map_cell(mx, my, 0, FIX_BLANK);
}

void hud_update_marker(void) {
    int px, py;
    if (!hud_map_on) return;
    rc_player_cell(&px, &py);
    if (px == prev_px && py == prev_py) return;
    if (prev_px >= 0) {
        if (map_at(prev_px, prev_py)) map_cell(prev_px, prev_py, PAL_MAP_WALL, FIX_SOLID);
        else                          map_cell(prev_px, prev_py, 0, FIX_BLANK);
    }
    map_cell(px, py, PAL_MAP_PLAYER, FIX_SOLID);
    prev_px = px; prev_py = py;
}

void hud_draw_int(u16 col, u16 row, u16 pal, int value, u8 width) {
    for (s16 i = width - 1; i >= 0; i--) {
        fix_poke((u16)(col + i), row, pal, (u16)(FIX_DIGIT_BASE + value % 10));
        value /= 10;
    }
}

void hud_draw_fps(int fps){
    hud_draw_int(36, 2, PAL_HUD, fps, 3);
}

/* takes total scannn lines to draw the time in tenths of ms*/
void hud_draw_time(u16 total_lines){
    hud_draw_int(35, 4, PAL_HUD, (total_lines * 10000) / 15840, 4);
}

void hud_init(void) {
    /* debug: set all indices white to find which index the font uses */
    for (u16 i = 1; i < 16; i++)
        pal_set(PAL_HUD, i, RGB(31, 31, 31));
}

void rom_callback_VBlank(void){
    vblank_count++;
}
