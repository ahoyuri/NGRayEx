
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

Interrupts (Cartridge):
Level 1: VBlank  (~60Hz)
Level 2: Timer   (konfigurierbar über REG_TIMERHIGH/TIMERLOW, 6MHz Pixelclock)
Level 3: Reset   (nur beim Kaltstart, vom BIOS behandelt)

ngdevkit Callbacks (Linker erkennt diese per Symbolname zur Link-Zeit):
  void rom_callback_VBlank(void)  -> wird bei jedem VBlank aufgerufen
  void rom_callback_Timer(void)   -> wird bei Timer-Interrupt aufgerufen
Keine Deklaration in einer .h nötig - der Linker sucht das Symbol direkt.
Kein Makro, kein Attribut, kein sei() wie beim ATmega nötig.

Callback = Funktion die man selbst schreibt, aber die das Framework/die Hardware aufruft.
Man definiert WAS passiert, das System entscheidet WANN (beim IRQ).

Atomarität auf dem 68000:
  u8  / u16  -> atomar (single bus cycle) -> sicher für IRQ-Variablen
  u32        -> NICHT atomar (2x 16-Bit) -> IRQ kann dazwischen feuern
  -> volatile u16 für IRQ-Zähler verwenden

VBlank-Counter für Renderzeit-Messung:
  u16 before = vblank_count;
  rc_render();
  u16 frames = vblank_count - before;
  // frames == 0: unter 1 Frame (kein Drop), frames >= 1: Frame(s) gedroppt
  // fps = 60 / (frames + 1)
