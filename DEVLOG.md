# NGRayEx — Dev Log

---

## 2026-06-11 — Floor renderer working, walls now render correctly on top

**Goal:** Fix render order so walls appear in front of the floor.

**Approach:** On the Neo Geo, higher sprite indices render in front. FLOOR_BASE
and WALL_BASE were swapped — floor sprites had higher indices than wall sprites.
Fix: reorder the defines in config.h so floor comes first (lower index).

**Result:** One-line change in config.h. Walls now correctly occlude the floor.

---

## 2026-06-12 - multiple sprites in one slice for higher resolution

**Goal:** A slice can render more then 15 pxl.

**Approche:** multiple sprites with there own color pallets in one slice.
- The Initial stepp of the FLOOR_SLICE needs to be variable. With the correct amount of pallets.
- The Pixel and color palet calculation checks in wich palet it needs to write.
- The amount of slices needs to be reduced from 80 to 64 to get 3 Sprites



<!--
Template for new entries:

## YYYY-MM-DD — Short title

**Goal:** What do I want to achieve?

**Approach:** How am I planning to do it?

**Problem:** What is blocking me / what is unclear?

**Result:** What worked, what didn't, what did I learn?

---
-->
