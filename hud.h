#ifndef HUD_H
#define HUD_H

#include "hw.h"

extern u8 hud_map_on;
extern volatile u16 vblank_count;


void hud_init(void);
void hud_draw_minimap(void);
void hud_clear_minimap(void);
void hud_update_marker(void);

/* Draw an integer right-aligned in a field of `width` digits. */
void hud_draw_int(u16 col, u16 row, u16 pal, int value, u8 width);
void hud_draw_fps(int fps);
void hud_draw_time(u16 total_lines);


#endif /* HUD_H */
