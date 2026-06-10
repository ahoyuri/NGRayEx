# NGRayEx — Floor Rendering on Neo Geo, with BSP as Next Target

A real-time, first-person renderer for the SNK Neo Geo AES, written in C.
The long-term goal is to replace the DDA raycaster with a proper BSP renderer
(similar to DOOM's approach), and to render fully textured floors and ceilings.

This was made purely for research purposes to understand the complexities of
rendering realtime "3D" on the Neo Geo.

---

## Current Status — Textured Floor via Per-Column Palette Encoding

The floor renderer is working visually. Each of the 80 screen columns gets its own
sprite with 15 stacked solid-color tiles. The 15 palette entries per column are set
every frame to encode 15 depth samples of the floor texture, sampled via perspective-
correct floor casting. A 3-level mip-map (LOD) reduces aliasing on far surfaces.

| | |
|---|---|
| ![corridor view](Screenshot_20260610_235831.png) | ![close wall](Screenshot_20260610_235927.png) |

**HUD (top-right corner):**
- **Top number** — FPS
- **Bottom number** — total render time of raycaster + floor render in **tenths of milliseconds**
  (e.g. `2164` = ~216 ms per frame)

Walls now correctly render in front of the floor (sprite index order fixed).

The floor caster uses a per-row step-increment model (identical to DOOM's
approach): only 15 world-space origins and 15 step vectors are computed per
column; all other samples within a row are derived by adding the step — no
division in the inner loop. The raycaster alone runs at ~7 FPS; combined with
the floor renderer the result is ~4 FPS (~216 ms/frame), which is acceptable
for this stage.

The clipping data (`floor_clip[0][c]` / `floor_clip[1][c]`) produced by the
raycaster is general-purpose — it marks the top and bottom of the visible floor
strip per column. The same mechanism would work unchanged for the **ceiling**;
only the floor renderer itself needs a second pass with mirrored sample positions.
Currently only the floor is rendered.

---

## Next Steps

1. **More floor resolution** — Each column currently has only 15 color bands
   (one per sprite tile). Increasing the band count would give smoother depth
   gradients and a finer texture appearance.

2. **Ceiling** — Mirror the floor pass: sample above the wall with flipped
   positions and write to a second set of per-column palette sprites. The
   clipping data is already produced by the raycaster.

3. **Replace raycaster with BSP renderer** — The DDA raycaster has inherent
   limitations (no non-axis-aligned walls, no sectors with height variation).
   The next step is to switch to a BSP tree + line-segment wall representation
   (DOOM-style), enabling portal rendering, variable floor/ceiling heights, and
   correct depth ordering.

4. **Sectors** — Variable floor and ceiling heights per room, requiring
   per-sector geometry data alongside the BSP tree.

5. **Optimize** — Profile the remaining bottlenecks (PALRAM write count,
   palette dirty-tracking, possible VBlank offload) once the feature set
   is more complete.

---

## How it works (current raycaster)

Every frame, for each of 80 screen columns:

1. Cast a ray through a 2D grid map until it hits a wall.
2. Measure the perpendicular distance and turn it into a slice height.
3. Write a vertical-shrink value, a Y position, and a palette into the sprite
   control block for that column's sprite.

The video chip then scales each column's brick-texture sprite to the computed
height. Floor and ceiling are a static backdrop of full-width sprites sitting
behind the walls (lower sprite indices draw first = further back). A HUD
minimap is drawn on the fix (text) layer, which always composites over sprites.

All arithmetic is 16.16 fixed-point. Rotation uses constant cos/sin multiplies.
The wall renderer writes only a few control words per column per frame; the
expensive pixel work is offloaded to the scaler hardware.

---

## Controls

| Input               | Action            |
|---------------------|-------------------|
| D-pad Up / Down     | Move forward/back |
| D-pad Left / Right  | Turn              |
| Hold A + Left/Right | Strafe            |
| C                   | Toggle minimap    |

---

## Building

Requires [ngdevkit](https://github.com/dciabrin/ngdevkit)

```sh
# graphics + sound ROMs (self-contained tile encoder)
python3 tools/gen_gfx.py

# compile and assemble the cartridge
make
```

`tools/gen_gfx.py` emits the C/S/M/V ROMs directly in the Neo Geo's planar
format, so the only ngdevkit dependency is the m68k toolchain. See the comments
at the top of each tool for details.

You must supply your own Neo Geo BIOS — it is copyrighted and not included.

Tested with [gngeo](https://github.com/dciabrin/gngeo). May not render correctly
on real hardware.

---

## Acknowledgements

Built against the [ngdevkit](https://github.com/dciabrin/ngdevkit) toolchain.
Hardware details cross-referenced from the
[Neo Geo Development Wiki](https://wiki.neogeodev.org).
