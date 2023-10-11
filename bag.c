#define BAG_LEN 98
#define UPCOMING_TILES_START 126
#define UPCOMING_TILES_SPRITE_START 1

#include <gb/gb.h>
#include <rand.h>
#include <string.h>

uint8_t new_bag[] =  {0,0,0,0,0,0,0,0,0,1,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4,4,4,4,4,5,5,6,6,6,7,7,8,8,8,8,8,8,8,8,8,9,10,11,11,11,11,12,12,13,13,13,13,13,13,14,14,14,14,14,14,14,14,15,15,16,17,17,17,17,17,17,18,18,18,18,19,19,19,19,19,19,20,20,20,20,21,21,22,22,23,24,24,25};
uint8_t bag[BAG_LEN*2];
uint8_t tiles_in_bag = 0;

void draw_bag(uint8_t *bag, uint8_t *new_bag)
{
    for (uint8_t i=BAG_LEN; i>1; i--) {
        uint8_t p = ((uint8_t)rand()) % (uint8_t)BAG_LEN;
        uint8_t temp = *(new_bag+i-1);
        *(new_bag+i-1) = *(new_bag+p);
        *(new_bag+p) = temp;
    }
    memcpy(bag + tiles_in_bag, new_bag, BAG_LEN);
    tiles_in_bag += BAG_LEN;
}

void print_bag(uint8_t *bag)
{
    for (uint8_t i = 0; i < BAG_LEN*2; i++)
    {
        EMU_printf("%c", (uint8_t) *(bag+i)+97);
        if (i+1!=BAG_LEN) EMU_printf("-");
    }
    EMU_printf("\n");
}

void set_upcoming(uint8_t *bag_pos)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        if(i%2 == 0) set_bkg_tile_xy(15 + i/2, 14, *(bag_pos+i) + UPCOMING_TILES_START);
        else
        {
            set_sprite_tile(i/2, *(bag_pos+i) + UPCOMING_TILES_SPRITE_START);
        }
    }
}

void shift_bag(uint8_t *bag_pos, uint8_t amount)
{
    memmove(bag_pos, bag_pos+amount, BAG_LEN*2-amount);
    tiles_in_bag -= amount;
    if (tiles_in_bag <= BAG_LEN)
    {
        draw_bag(bag, new_bag);
    }
}