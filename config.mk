# Auto-generated config file for ngdevkit-examples
# This file only holds configuration for all the build rules
# It expects ngdevkit binary to be in your PATH
# Re-generate with ./configure

# Detect OS: Windows sets OS=Windows_NT (includes MSYS2/UCRT64)
ifeq ($(OS),Windows_NT)
PREFIX=/ucrt64
else
PREFIX=/usr
endif

# ngdevkit dependencies
PKGCONFIG=$(PREFIX)/bin/pkg-config
PYTHON=$(PREFIX)/bin/python3
ZIP=$(PREFIX)/bin/zip

# ngdevkit toolchain
M68KAR=$(PREFIX)/bin/m68k-neogeo-elf-ar
M68KAS=$(PREFIX)/bin/m68k-neogeo-elf-as
M68KGCC=$(PREFIX)/bin/m68k-neogeo-elf-gcc
M68KGXX=$(PREFIX)/bin/m68k-neogeo-elf-g++
M68KLD=$(PREFIX)/bin/m68k-neogeo-elf-ld
M68KOBJCOPY=$(PREFIX)/bin/m68k-neogeo-elf-objcopy
M68KRANLIB=$(PREFIX)/bin/m68k-neogeo-elf-ranlib
Z80SDAR=$(PREFIX)/bin/z80-neogeo-ihx-sdar
Z80SDAS=$(PREFIX)/bin/z80-neogeo-ihx-sdasz80
Z80SDCC=$(PREFIX)/bin/z80-neogeo-ihx-sdcc
Z80SDLD=$(PREFIX)/bin/z80-neogeo-ihx-sdldz80
Z80SDOBJCOPY=$(PREFIX)/bin/z80-neogeo-ihx-sdobjcopy
Z80SDRANLIB=$(PREFIX)/bin/z80-neogeo-ihx-sdranlib

# ngdevkit tools
PALTOOL=$(PREFIX)/bin/paltool.py
TILETOOL=$(PREFIX)/bin/tiletool.py
ADPCMTOOL=$(PREFIX)/bin/adpcmtool.py
VROMTOOL=$(PREFIX)/bin/vromtool.py
FURTOOL=$(PREFIX)/bin/furtool.py
NSSTOOL=$(PREFIX)/bin/nsstool.py
SOUNDTOOL=$(PREFIX)/bin/soundtool.py

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
CONVERT=$(PREFIX)/bin/convert

# any additional config or dependencies can be added below
SOX=$(PREFIX)/bin/sox
RSYNC=$(PREFIX)/bin/rsync
GNGEO=$(PREFIX)/bin/ngdevkit-gngeo

# GnGeo config
GNGEO_GLSL=yes
EXTRAOPTS=--p1control="A=K122,B=K120,C=K97,D=K115,START=K49,COIN=K51,UP=K82,DOWN=K81,LEFT=K80,RIGHT=K79,MENU=K27"

ifeq ($(OS),Windows_NT)
GNGEO_SHADER_PATH=
GLSL_SHADER_PATH=
SHADER_PATH=
SHADER=
else
GNGEO_SHADER_PATH=
GLSL_SHADER_PATH=
SHADER_PATH=
SHADER=
endif

# OS-specific
ifeq ($(OS),Windows_NT)
ENABLE_MSYS2=yes
else
ENABLE_MSYS2=no
endif
ENABLE_MINGW=no
GNGEO_INSTALL_PATH=
