#include <gb/gb.h>
#include <gb/emu_debug.h>
#include "dict.h"

#define MORE_MASK 0x800000
#define LETTER_MASK 0x7C0000
#define END_MASK 0x20000
#define INDEX_MASK 0x1FFFF

uint8_t is_word(unsigned char* word, uint8_t len)
{
    uint8_t l = 0; // word letter index
    uint32_t i = 0; // dictionary entry index

    while (1)
    {
        const uint8_t *dict;

        // switch to correct rom bank and set dict
        dict_switch(&dict, i);

        uint8_t c = word[l]-97;
        uint32_t entry = ((uint32_t) *(dict) << 16) | ((uint32_t) *(++dict) << 8) | *(++dict);

        uint8_t more = ((entry & MORE_MASK) >> 23);
        uint8_t letter = ((entry & LETTER_MASK) >> 18);
        uint8_t end = ((entry & END_MASK) >> 17);
        uint32_t index = ((entry & INDEX_MASK));

        if (letter == c && (end) == (l == len-1))
        {
            if (end) return 1;
            i = index;
            l++;
        }
        else if (more) i++;
        else return 0;
    }
}