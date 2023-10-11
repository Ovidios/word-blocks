#include <gb/gb.h>

#define BAG_STEP 4904

extern const uint8_t dict_0[]; // in ROM bank 1
extern const uint8_t dict_1[]; // in ROM bank 2
extern const uint8_t dict_2[]; // in ROM bank 3
extern const uint8_t dict_3[]; // in ROM bank 4
extern const uint8_t dict_4[]; // in ROM bank 5
extern const uint8_t dict_5[]; // in ROM bank 6
extern const uint8_t dict_6[]; // in ROM bank 7
extern const uint8_t dict_7[]; // in ROM bank 8
extern const uint8_t dict_8[]; // in ROM bank 10
extern const uint8_t dict_9[]; // in ROM bank 11
extern const uint8_t dict_10[]; // in ROM bank 12
extern const uint8_t dict_11[]; // in ROM bank 13

void dict_switch(const uint8_t** d, uint32_t i)
{
    if (i  < (uint16_t)BAG_STEP)
    {
        SWITCH_ROM(1);
        *d = &dict_0[0];
        *d += i*3;
    }
    else if (i < (uint16_t)BAG_STEP*2)
    {
        SWITCH_ROM(2);
        *d = &dict_1[0];
        *d += (i - (uint16_t)BAG_STEP)*3;
    }
    else if (i < (uint16_t)BAG_STEP*3)
    {
        SWITCH_ROM(3);
        *d = &dict_2[0];
        *d += (i - BAG_STEP*2)*3;
    }
    else if (i < (uint16_t)BAG_STEP*4)
    {
        SWITCH_ROM(4);
        *d = &dict_3[0];
        *d += (i - (uint16_t)BAG_STEP*3)*3;
    }
    else if (i < (uint16_t)BAG_STEP*5)
    {
        SWITCH_ROM(5);
        *d = &dict_4[0];
        *d += (i - (uint16_t)BAG_STEP*4)*3;
    }
    else if (i < (uint16_t)BAG_STEP*6)
    {
        SWITCH_ROM(6);
        *d = &dict_5[0];
        *d += (i - (uint16_t)BAG_STEP*5)*3;
    }
    else if (i < (uint16_t)BAG_STEP*7)
    {
        SWITCH_ROM(7);
        *d = &dict_6[0];
        *d += (i - (uint16_t)BAG_STEP*6)*3;
    }
    else if (i < (uint16_t)BAG_STEP*8)
    {
        SWITCH_ROM(8);
        *d = &dict_7[0];
        *d += (i - (uint16_t)BAG_STEP*7)*3;
    }
    else if (i < (uint16_t)BAG_STEP*9)
    {
        SWITCH_ROM(10);
        *d = &dict_8[0];
        *d += (i - (uint16_t)BAG_STEP*8)*3;
    }
    else if (i < (uint16_t)BAG_STEP*10)
    {
        SWITCH_ROM(11);
        *d = &dict_9[0];
        *d += (i - (uint16_t)BAG_STEP*9)*3;
    }
    else if (i < (uint16_t)BAG_STEP*11)
    {
        SWITCH_ROM(12);
        *d = &dict_10[0];
        *d += (i - (uint16_t)BAG_STEP*10)*3;
    }
    else
    {
        SWITCH_ROM(13);
        *d = &dict_11[0];
        *d += (i - (uint16_t)BAG_STEP*11)*3;
    }
}