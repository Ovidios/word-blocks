#include <gb/gb.h>
#include <gb/cgb.h>

#define MAX_PAL 4

uint8_t curr_pal = 0;

uint16_t bgPalette[16];
uint16_t spritePalette[20];

const uint16_t palettes[] = {
    0x67bf,0x5adf,0x269c,0x25b8,0x20ad,
    0x67bf,0x2d7f,0x127d,0x1534,0x1405,
    0x7ff9,0x4fd6,0x7e87,0x592d,0x2489,
    0x7fff,0x6739,0x4210,0x318c,0x0000,
    0x63f8,0x5bdf,0x6d3c,0x6168,0x31a5
};

void switch_palette()
{
    // bg/window palettes
    bgPalette[0] = palettes[curr_pal*5 + 0];
    bgPalette[1] = palettes[curr_pal*5 + 2];
    bgPalette[2] = palettes[curr_pal*5 + 3];
    bgPalette[3] = palettes[curr_pal*5 + 4];

    bgPalette[4] = palettes[curr_pal*5 + 0];
    bgPalette[5] = palettes[curr_pal*5 + 1];
    bgPalette[6] = palettes[curr_pal*5 + 2];
    bgPalette[7] = palettes[curr_pal*5 + 3];

    bgPalette[8] =  palettes[curr_pal*5 + 0];
    bgPalette[9] =  palettes[curr_pal*5 + 1];
    bgPalette[10] = palettes[curr_pal*5 + 3];
    bgPalette[11] = palettes[curr_pal*5 + 4];

    bgPalette[12] = palettes[curr_pal*5 + 4];
    bgPalette[13] = palettes[curr_pal*5 + 4];
    bgPalette[14] = palettes[curr_pal*5 + 4];
    bgPalette[15] = palettes[curr_pal*5 + 4];

    // sprite palettes
    spritePalette[1] = palettes[curr_pal*5 + 0];
    spritePalette[2] = palettes[curr_pal*5 + 3];
    spritePalette[3] = palettes[curr_pal*5 + 4];

    spritePalette[5] = palettes[curr_pal*5 + 1];
    spritePalette[6] = palettes[curr_pal*5 + 2];
    spritePalette[7] = palettes[curr_pal*5 + 4];

    spritePalette[9]  = palettes[curr_pal*5 + 2];
    spritePalette[10] = palettes[curr_pal*5 + 2];
    spritePalette[11] = palettes[curr_pal*5 + 2];

    spritePalette[13] = palettes[curr_pal*5 + 0];
    spritePalette[14] = palettes[curr_pal*5 + 3];
    spritePalette[15] = palettes[curr_pal*5 + 4];

    spritePalette[17] = palettes[curr_pal*5 + 4];
    spritePalette[18] = palettes[curr_pal*5 + 4];
    spritePalette[19] = palettes[curr_pal*5 + 3];

    set_bkg_palette(0, 4, &bgPalette[0]);
    set_sprite_palette(0, 5, &spritePalette[0]);
}