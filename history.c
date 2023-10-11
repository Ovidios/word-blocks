#define HIST_LETTERS_START 162

#include <gb/gb.h>

void write_word_to_hist(char* word, uint8_t len)
{
    // move other words down
    uint8_t mov_tiles[6];

    get_bkg_tiles(13, 10, 6, 1, &mov_tiles[0]);
    set_bkg_tiles(13, 11, 6, 1, &mov_tiles[0]);

    get_bkg_tiles(13, 9, 6, 1, &mov_tiles[0]);
    set_bkg_tiles(13, 10, 6, 1, &mov_tiles[0]);

    get_bkg_tiles(13, 8, 6, 1, &mov_tiles[0]);
    set_bkg_tiles(13, 9, 6, 1, &mov_tiles[0]);

    get_bkg_tiles(13, 7, 6, 1, &mov_tiles[0]);
    set_bkg_tiles(13, 8, 6, 1, &mov_tiles[0]);

    get_bkg_tiles(13, 6, 6, 1, &mov_tiles[0]);
    set_bkg_tiles(13, 7, 6, 1, &mov_tiles[0]);

    get_bkg_tiles(13, 5, 6, 1, &mov_tiles[0]);
    set_bkg_tiles(13, 6, 6, 1, &mov_tiles[0]);
    
    // write new word
    for (uint8_t i = 0; i<6; i++)
    {
        if(i<len) set_bkg_tile_xy(13+i, 5, word[i] + HIST_LETTERS_START - 97);
        else set_bkg_tile_xy(13+i, 5, 1);
    }
}