#!/usr/bin/env python3
"""gen_gfx.py - emit the non-program ROMs for the raycaster cart.

Outputs into ./rom :
  c1.bin / c2.bin   sprite tiles (the wall brick, a solid tile, a blank tile)
  s1.bin            fix layer ROM (all-blank: we never draw the fix layer)
  m1.bin / v1.bin   silent sound placeholders

C-ROM encoding follows the NeoGeo dev wiki "Sprite graphics format":
a 16x16 tile is four 8x8 blocks in order (8,0)(8,8)(0,0)(0,8); per row,
bitplanes 0,1 go to the odd ROM (C1) and 2,3 to the even ROM (C2); the
leftmost pixel is the MSB; the color index MSB is bitplane 3.
"""
import os

C_PAD = 0x80000   # pad each C ROM to 512 KiB (well above any tile we use)
S_PAD = 0x20000   # 128 KiB fix ROM, all blank
M_PAD = 0x10000   # 64 KiB Z80 program, all 0x00 (NOP) -> silent
V_PAD = 0x10000   # 64 KiB ADPCM samples, empty


def encode_tile(px):
    """px: 16x16 list-of-lists of palette indices (0..15) -> (c1, c2) bytes."""
    c1, c2 = bytearray(), bytearray()
    for (xo, yo) in ((8, 0), (8, 8), (0, 0), (0, 8)):
        for row in range(8):
            y = yo + row
            p0 = p1 = p2 = p3 = 0
            for pix in range(8):
                ci = px[y][xo + pix]
                bit = 7 - pix
                p0 |= ((ci >> 0) & 1) << bit
                p1 |= ((ci >> 1) & 1) << bit
                p2 |= ((ci >> 2) & 1) << bit
                p3 |= ((ci >> 3) & 1) << bit
            c1 += bytes((p0, p1))
            c2 += bytes((p2, p3))
    return c1, c2


def tile_blank():
    return [[0] * 16 for _ in range(16)]


def tile_solid():
    return [[1] * 16 for _ in range(16)]


def tile_brick():
    """Running-bond brick: index 1 mortar, 2 brick body, 3 top highlight."""
    px = [[2] * 16 for _ in range(16)]
    for y in range(16):
        course = y // 8                      # two courses per tile
        offset = 0 if course == 0 else 8     # half-brick stagger
        for x in range(16):
            if y % 8 == 0:                   # horizontal mortar
                px[y][x] = 1
            elif (x + offset) % 8 == 0:      # vertical mortar
                px[y][x] = 1
            elif y % 8 == 1:                 # lit top edge of each brick
                px[y][x] = 3
    return px


def main():
    here = os.path.dirname(os.path.abspath(__file__))
    out = os.path.join(here, "..", "rom")
    os.makedirs(out, exist_ok=True)

    tiles = [tile_blank(), tile_brick(), tile_solid()]   # indices 0,1,2
    c1, c2 = bytearray(), bytearray()
    for t in tiles:
        a, b = encode_tile(t)
        c1 += a
        c2 += b
    assert len(c1) == len(tiles) * 64 and len(c2) == len(tiles) * 64

    c1 += bytes(C_PAD - len(c1))
    c2 += bytes(C_PAD - len(c2))

    # Fix (S-ROM) tileset: tile 0 = transparent (all index 0 -> all 0x00),
    # tile 1 = solid (all index 15 -> all 0xFF). The all-0xFF tile reads as
    # color index 15 regardless of the fix format's byte/plane ordering, so
    # the minimap picks its colour purely via the fix word's palette field.
    FIX_TILE = 32  # 8x8 * 4bpp = 32 bytes
    s1 = bytearray()
    s1 += bytes(FIX_TILE)            # tile 0: blank
    s1 += bytes([0xFF]) * FIX_TILE   # tile 1: solid index 15
    s1 += bytes(S_PAD - len(s1))

    files = {
        "c1.bin": c1,
        "c2.bin": c2,
        "s1.bin": s1,
        "m1.bin": bytes(M_PAD),
        "v1.bin": bytes(V_PAD),
    }
    for name, data in files.items():
        with open(os.path.join(out, name), "wb") as f:
            f.write(data)
        print(f"  {name:8} {len(data):#8x} bytes")

    print(f"  tiles encoded: blank={len(tiles)>0} brick solid "
          f"({len(tiles)*128} C-ROM bytes used)")


if __name__ == "__main__":
    main()
