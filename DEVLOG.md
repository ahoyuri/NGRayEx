
run command with resizable window:
ngdevkit-gngeo -b glsl --scale 4 -i build/rom puzzledp

Fix Layer VRAM-Bereich:
Start:  $7000
End:    $74FF
Größe:  1280 Wörter (40 x 32)

spaltenweise gemappt (top->bottom, left->right)
Adresse = $7000 + col * 32 + row (siehe hw.h fix_poke)


SCB = Sprite Control Block
SCB1 Tile-Daten: welche Tiles + welche Palette
SCB2 Zoom/shrink: horizontale & vertikale Skalierung
SCB3 Y-Position + Sticky-Bit + Höhe (in Tiles)
SCB4 X-Position

SCB2-4 sind 512Wörter groß, SCB1 = 32.768 Wörter. Sprites bis zu 64 Tiles.
sticky bit  dann erbt das nächste Sprote die X-Position und Zoom vom vorherigen.
So baut man breite Objekte aus mehreren Sprite Streifen zusammen.

Fix Layer:
8x8-Pixel-Tiles diese liegen im S-ROM
rom/s1.bin
4-Bit farbtiefe Index 0 = transparent
-> GIF kommt in den assets/ Ordner und muss dann in der Makefile als Abhängigkeit von $(SROM1) ein
