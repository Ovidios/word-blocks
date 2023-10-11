#include <gb/gb.h>

enum sounds {MOVE, DROP};

extern uint8_t sound;

void play_sound(enum sounds sound_n)
{
    if (!sound) return;

    NR52_REG = 0x80;
    NR51_REG = 0x11;
    NR50_REG = 0x77;

    switch (sound_n)
    {
        case MOVE:
            NR10_REG = 0x00;
            NR11_REG = 0x81;
            NR12_REG = 0x23;
            NR13_REG = 0x32;
            NR14_REG = 0x87;
            break;
        case DROP:
            NR10_REG = 0x1B;
            NR11_REG = 0xC0;
            NR12_REG = 0x43;
            NR13_REG = 0xA7;
            NR14_REG = 0xC5;
            break;
    }
}