/* raycast.h */
#ifndef RAYCAST_H
#define RAYCAST_H

#include "hw.h"
#include "config.h"

/* ---- 16.16 fixed point ------------------------------------------------- */
typedef s32 fix;
#define FBITS 16
#define FONE  (1 << FBITS)
#define FIX(x) ((fix)((x) * (double)FONE))   /* constant initializers only  */

static inline fix fmul(fix a, fix b)  { return (fix)(((int64_t)a * b) >> FBITS); }
static inline fix fdiv(fix a, fix b)  { return (fix)(((int64_t)a << FBITS) / b); }
static inline fix fabsx(fix a)        { return a < 0 ? -a : a; }

void rc_init(void);          /* set player start, init shadow buffers       */
void rc_input(u8 pressed);   /* pressed = active-HIGH P1 bits (already inv.) */
void rc_render(void);        /* run DDA for every column -> shadow buffers   */
void rc_blit(void);          /* push shadow buffers to VRAM (call in vblank) */
void rc_player_cell(int *cx, int *cy); /* current player grid cell (out params) */
void rc_camera(fix *px, fix *py, fix *dx, fix *dy, fix *plx, fix *ply);

/* [0][c]=floor top (wall bottom), [1][c]=floor bottom (screen bottom) */
extern u16 floor_clip[2][NUM_COLS];

#endif /* RAYCAST_H */
