# Auto-generated config file for ngdevkit-examples
# This file only holds configuration for all the build rules
# It expects ngdevkit binary to be in your PATH
# Re-generate with ./configure

# ngdevkit dependencies
PKGCONFIG=/usr/bin/pkg-config
PYTHON=/usr/bin/python3
ZIP=/usr/bin/zip

# ngdevkit toolchain
M68KAR=/usr/bin/m68k-neogeo-elf-ar
M68KAS=/usr/bin/m68k-neogeo-elf-as
M68KGCC=/usr/bin/m68k-neogeo-elf-gcc
M68KGXX=/usr/bin/m68k-neogeo-elf-g++
M68KLD=/usr/bin/m68k-neogeo-elf-ld
M68KOBJCOPY=/usr/bin/m68k-neogeo-elf-objcopy
M68KRANLIB=/usr/bin/m68k-neogeo-elf-ranlib
Z80SDAR=/usr/bin/z80-neogeo-ihx-sdar
Z80SDAS=/usr/bin/z80-neogeo-ihx-sdasz80
Z80SDCC=/usr/bin/z80-neogeo-ihx-sdcc
Z80SDLD=/usr/bin/z80-neogeo-ihx-sdldz80
Z80SDOBJCOPY=/usr/bin/z80-neogeo-ihx-sdobjcopy
Z80SDRANLIB=/usr/bin/z80-neogeo-ihx-sdranlib

# ngdevkit tools
PALTOOL=/usr/bin/paltool.py
TILETOOL=/usr/bin/tiletool.py
ADPCMTOOL=/usr/bin/adpcmtool.py
VROMTOOL=/usr/bin/vromtool.py
FURTOOL=/usr/bin/furtool.py
NSSTOOL=/usr/bin/nsstool.py
SOUNDTOOL=/usr/bin/soundtool.py

# ngdevkit build flags
NGCFLAGS=`$(PKGCONFIG) --cflags ngdevkit`
NGLDFLAGS=`$(PKGCONFIG) --libs ngdevkit`
NGLIBDIR=`$(PKGCONFIG) --variable=libdir ngdevkit`
NGZ80INCLUDEDIR=`$(PKGCONFIG) --variable=z80includedir ngdevkit`
NGZ80LIBDIR=`$(PKGCONFIG) --variable=z80libdir ngdevkit`
# this variable must be resolved here as it is used as a base
# directory for dependencies (nullsound, nullbios)
NGSHAREDIR=$(shell $(PKGCONFIG) --variable=sharedir ngdevkit)

# additional dependencies
CONVERT=/usr/bin/convert

# any additional config or dependencies can be added below
SOX=/usr/bin/sox
RSYNC=/usr/bin/rsync
GNGEO=/usr/bin/ngdevkit-gngeo

# GnGeo config
GNGEO_GLSL=yes
GNGEO_SHADER_PATH=/usr/share/ngdevkit-gngeo
GLSL_SHADER_PATH=/root/examples/shaders
SHADER_PATH=/root/examples/shaders
SHADER=qcrt-flat.glslp

# OS-specific
ENABLE_MSYS2=no
ENABLE_MINGW=no
GNGEO_INSTALL_PATH=
