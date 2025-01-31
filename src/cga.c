//
// Created by xrip on 30.10.2023.
//
#include "cga.h"

uint8_t cursor_blink_state = 0;
uint8_t cga_intensity = 0;
uint8_t cga_colorset = 0;

const uint32_t cga_palette[16] = {
    //R, G, B
    0x000000, //black
    0x0000C4, //blue
    0x00C400, //green
    0x00C4C4, //cyan
    0xC40000, //red
    0xC400C4, //magenta
    0xC47E00, //brown
    0xC4C4C4, //light gray
    0x4E4E4E, //dark gray
    0x4E4EDC, //light blue
    0x4EDC4E, //light green
    0x4EF3F3, //light cyan
    0xDC4E4E, //light red
    0xF34EF3, //light magenta
    0xF3F34E, //yellow
    0xFFFFFF //white
};

// cga 16 shade palette (text mode)
const uint32_t cga_grayscale_palette[16] = {
    0x000000,
    0x393939,
    0x818181,
    0x8d8d8d,
    0x5c5c5c,
    0x6c6c6c,
    0x717171,
    0xa9a9a9,
    0x545454,
    0x757575,
    0xcacaca,
    0xd9d9d9,
    0x9b9b9b,
    0xafafaf,
    0xf0f0f0,
    0xfefefe,
  };

const uint8_t cga_gfxpal[3][2][4] = {
    //palettes for 320x200 graphics mode
    {
        { 0, 2, 4, 6 }, //normal palettes
        { 0, 3, 5, 7 }
    },
    {
        { 0, 10, 12, 14 }, //intense palettes
        { 0, 11, 13, 15 }
    },
    {
        // the unofficial Mode 5 palette, accessed by disabling ColorBurst
        { 0, 3, 4, 7 },
        { 0, 11, 12, 15 },
    },
};

const uint32_t cga_composite_palette[3][16] = {
    //R, G, B
    // 640x200 Color Composite
    {
        0x000000, // black
        0x007100, // d.green
        0x003fff, // d.blue
        0x00abff, // m.blue
        0xc10065, // red
        0x737373, // gray
        0xe639ff, // purple
        0x8ca8ff, // l.blue
        0x554600, // brown
        0x00cd00, // l.green
        0x00cd00, // gray 2
        0x00fc7e, // aqua
        0xff3900, // orange
        0xe4cc00, // yellow
        0xff7af2, // pink
        0xffffff //white
    },
    {
        // 320x200 Palette 0 High Intensity Composite Color
        0x000000, //
        0x00766d, //
        0x00316f, //
        0x7b3400, //
        0x39be42, //
        0x837649, //
        0x539b0e, //
        0xeb3207, //
        0xd2c499, //
        0xf87a9b, //
        0xd9a06b, //
        0xb34400, //
        0x8bd04a, //
        0xbe8550, //
        0x98ad14, //
        0x000000 // ???
    },
    {
        // 320x200 Palette 1 High Intensity Composite Color
        0x000000, //
        0x008bac, //
        0x0049ae, //
        0x009ee8, //
        0x581c00, //
        0x00bc9b, //
        0x64759f, //
        0x00cdd9, //
        0xc81b26, //
        0xb2c2ec, //
        0xdd7def, //
        0xbed3ff, //
        0xff4900, //
        0xf6edc0, //
        0xffa4c3, //
        0xffffff //
    }
};

const uint32_t tandy_palette[16] = {
    0x000000, // Black
    0x0000AA, // Dark Blue
    0x00AA00, // Dark Green
    0x00AAAA, // Teal
    0xAA0000, // Dark Red
    0xAA00AA, // Purple
    0xAA5500, // Brown
    0xAAAAAA, // Light Gray
    0x555555, // Dark Gray
    0x5555FF, // Blue
    0x55FF55, // Green
    0x55FFFF, // Aqua
    0xFF5555, // Red
    0xFF55FF, // Magenta
    0xFFFF55, // Yellow
    0xFFFFFF // White
};

uint32_t vga_palette[256] = {
(0x000000),
(0x00002A),
(0x002A00),
(0x002A2A),
(0x2A0000),
(0x2A002A),
(0x2A1500),
(0x2A2A2A),
(0x151515),
(0x15153F),
(0x153F15),
(0x153F3F),
(0x3F1515),
(0x3F153F),
(0x3F3F15),
(0x3F3F3F),
(0x000000),
(0x050505),
(0x080808),
(0x0B0B0B),
(0x0E0E0E),
(0x111111),
(0x141414),
(0x181818),
(0x1C1C1C),
(0x202020),
(0x242424),
(0x282828),
(0x2D2D2D),
(0x323232),
(0x383838),
(0x3F3F3F),
(0x00003F),
(0x10003F),
(0x1F003F),
(0x2F003F),
(0x3F003F),
(0x3F002F),
(0x3F001F),
(0x3F0010),
(0x3F0000),
(0x3F1000),
(0x3F1F00),
(0x3F2F00),
(0x3F3F00),
(0x2F3F00),
(0x1F3F00),
(0x103F00),
(0x003F00),
(0x003F10),
(0x003F1F),
(0x003F2F),
(0x003F3F),
(0x002F3F),
(0x001F3F),
(0x00103F),
(0x1F1F3F),
(0x271F3F),
(0x2F1F3F),
(0x371F3F),
(0x3F1F3F),
(0x3F1F37),
(0x3F1F2F),
(0x3F1F27),
(0x3F1F1F),
(0x3F271F),
(0x3F2F1F),
(0x3F371F),
(0x3F3F1F),
(0x373F1F),
(0x2F3F1F),
(0x273F1F),
(0x1F3F1F),
(0x1F3F27),
(0x1F3F2F),
(0x1F3F37),
(0x1F3F3F),
(0x1F373F),
(0x1F2F3F),
(0x1F273F),
(0x2D2D3F),
(0x312D3F),
(0x362D3F),
(0x3A2D3F),
(0x3F2D3F),
(0x3F2D3A),
(0x3F2D36),
(0x3F2D31),
(0x3F2D2D),
(0x3F312D),
(0x3F362D),
(0x3F3A2D),
(0x3F3F2D),
(0x3A3F2D),
(0x363F2D),
(0x313F2D),
(0x2D3F2D),
(0x2D3F31),
(0x2D3F36),
(0x2D3F3A),
(0x2D3F3F),
(0x2D3A3F),
(0x2D363F),
(0x2D313F),
(0x00001C),
(0x07001C),
(0x0E001C),
(0x15001C),
(0x1C001C),
(0x1C0015),
(0x1C000E),
(0x1C0007),
(0x1C0000),
(0x1C0700),
(0x1C0E00),
(0x1C1500),
(0x1C1C00),
(0x151C00),
(0x0E1C00),
(0x071C00),
(0x001C00),
(0x001C07),
(0x001C0E),
(0x001C15),
(0x001C1C),
(0x00151C),
(0x000E1C),
(0x00071C),
(0x0E0E1C),
(0x110E1C),
(0x150E1C),
(0x180E1C),
(0x1C0E1C),
(0x1C0E18),
(0x1C0E15),
(0x1C0E11),
(0x1C0E0E),
(0x1C110E),
(0x1C150E),
(0x1C180E),
(0x1C1C0E),
(0x181C0E),
(0x151C0E),
(0x111C0E),
(0x0E1C0E),
(0x0E1C11),
(0x0E1C15),
(0x0E1C18),
(0x0E1C1C),
(0x0E181C),
(0x0E151C),
(0x0E111C),
(0x14141C),
(0x16141C),
(0x18141C),
(0x1A141C),
(0x1C141C),
(0x1C141A),
(0x1C1418),
(0x1C1416),
(0x1C1414),
(0x1C1614),
(0x1C1814),
(0x1C1A14),
(0x1C1C14),
(0x1A1C14),
(0x181C14),
(0x161C14),
(0x141C14),
(0x141C16),
(0x141C18),
(0x141C1A),
(0x141C1C),
(0x141A1C),
(0x14181C),
(0x14161C),
(0x000010),
(0x040010),
(0x080010),
(0x0C0010),
(0x100010),
(0x10000C),
(0x100008),
(0x100004),
(0x100000),
(0x100400),
(0x100800),
(0x100C00),
(0x101000),
(0x0C1000),
(0x081000),
(0x041000),
(0x001000),
(0x001004),
(0x001008),
(0x00100C),
(0x001010),
(0x000C10),
(0x000810),
(0x000410),
(0x080810),
(0x0A0810),
(0x0C0810),
(0x0E0810),
(0x100810),
(0x10080E),
(0x10080C),
(0x10080A),
(0x100808),
(0x100A08),
(0x100C08),
(0x100E08),
(0x101008),
(0x0E1008),
(0x0C1008),
(0x0A1008),
(0x081008),
(0x08100A),
(0x08100C),
(0x08100E),
(0x081010),
(0x080E10),
(0x080C10),
(0x080A10),
(0x0B0B10),
(0x0C0B10),
(0x0D0B10),
(0x0F0B10),
(0x100B10),
(0x100B0F),
(0x100B0D),
(0x100B0C),
(0x100B0B),
(0x100C0B),
(0x100D0B),
(0x100F0B),
(0x10100B),
(0x0F100B),
(0x0D100B),
(0x0C100B),
(0x0B100B),
(0x0B100C),
(0x0B100D),
(0x0B100F),
(0x0B1010),
(0x0B0F10),
(0x0B0D10),
(0x0B0C10),
(0x000000),
(0x000000),
(0x000000),
(0x000000),
(0x000000),
(0x000000),
(0x000000),
(0x000000)
};
