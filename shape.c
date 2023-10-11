#define UPCOMING_TILES_START 126
#define PLACED_TILES_START 100
#define SHAPES_SPRITE_START 27
#define HD_INDICATOR_START 1
#define SHAPE_SPAWN_X 5
#define SHAPE_SPAWN_Y 1

#include <gb/gb.h>
#include <rand.h>
#include <string.h>
#include <gb/emu_debug.h>

extern uint8_t drop_timer;

extern uint8_t is_word(unsigned char* word, uint8_t len);

extern void add_score(uint8_t len);
extern void update_score();
extern void set_gamestate(uint8_t state);

uint16_t neighbor;

/* SHAPES ENUM */
enum shapes {
    SINGLE,
    DOUBLE_1, DOUBLE_2, DOUBLE_3, DOUBLE_4,
    L_1, L_2, L_3, L_4,
    BLOCK_1, BLOCK_2, BLOCK_3, BLOCK_4
};
enum shapes shape_bag[] = {SINGLE, DOUBLE_1, L_1, BLOCK_1};

/* SHAPE LAYOUT LOOKUP */
uint8_t shape_sprit_num[] = {
    0, 8, 8, 8, // SINGLE
    0, 1, 8, 8, // DOUBLE_1
    0, 8, 1, 8, // DOUBLE_2
    1, 0, 8, 8, // DOUBLE_3
    1, 8, 0, 8, // DOUBLE_4
    0, 1, 2, 8, // L_1
    2, 0, 8, 1, // L_2
    8, 2, 1, 0, // L_3
    1, 8, 0, 2, // L_4
    0, 1, 2, 3, // BLOCK_1
    2, 0, 3, 1, // BLOCK_2
    3, 2, 1, 0, // BLOCK_3
    1, 3, 0, 2  // BLOCK_4
};
uint8_t shape_width[] = {
    0, 1,0,1,0, 1,1,1,1, 1,1,1,1
};
uint8_t shape_height[] = {
    0, 0,1,0,1, 1,1,1,1, 1,1,1,1
};
uint8_t shape_letter_count[] = {1,2,2,2,2,3,3,3,3,4,4,4,4};

/* SHAPE ROTATION LOOKUP */
enum shapes rotate_cw[] = {
    SINGLE,
    DOUBLE_2, DOUBLE_3, DOUBLE_4, DOUBLE_1,
    L_2, L_3, L_4, L_1,
    BLOCK_2, BLOCK_3, BLOCK_4, BLOCK_1,
};
enum shapes rotate_ccw[] = {
    SINGLE,
    DOUBLE_4, DOUBLE_1, DOUBLE_2, DOUBLE_3,
    L_4, L_1, L_2, L_3,
    BLOCK_4, BLOCK_1, BLOCK_2, BLOCK_3,
};

/* SHAPE COLLISION CHECK DATA */
enum col_dir {LEFT, RIGHT, UP, DOWN};
uint16_t shape_col_check[] = {
    // LEFT        RIGHT          UP              DOWN
    0b001000000000,0b000010000000,0b100000000000,0b000000010000,   // SINGLE
    0b001000000000,0b000001000000,0b110000000000,0b000000011000,   // DOUBLE_1
    0b001000100000,0b000010001000,0b100000000000,0b000000000010,   // DOUBLE_2
    0b001000000000,0b000001000000,0b110000000000,0b000000011000,   // DOUBLE_3
    0b001000100000,0b000010001000,0b100000000000,0b000000000010,   // DOUBLE_4
    0b001000100000,0b000001001000,0b110000000000,0b000000001010,   // L_1
    0b001000010000,0b000001000100,0b110000000000,0b000000010001,   // L_2
    0b000100100000,0b000001000100,0b010100000000,0b000000000011,   // L_3
    0b001000100000,0b000010000100,0b100010000000,0b000000000011,   // L_4
    0b001000100000,0b000001000100,0b110000000000,0b000000000011,   // BLOCK_1
    0b001000100000,0b000001000100,0b110000000000,0b000000000011,   // BLOCK_2
    0b001000100000,0b000001000100,0b110000000000,0b000000000011,   // BLOCK_3
    0b001000100000,0b000001000100,0b110000000000,0b000000000011    // BLOCK_4
};
uint8_t shape_rot_check[] = {
    0b1000,                         // SINGLE
    0b1100,0b1010,0b1100,0b1010,    // DOUBLE
    0b1110,0b1101,0b0111,0b1011,    // L
    0b1111,0b1111,0b1111,0b1111     // BLOCK
};

/* CURRENT/NEXT SHAPE DATA */
uint8_t shape_x = 31;
uint8_t shape_y = 31;
uint8_t shape_type = L_1;
enum shapes next_shape_type = SINGLE;
uint8_t shape_letters[] = {0, 1, 2, 3};
uint8_t is_on_ground = 0;

/* OTHER STUFF */
uint8_t next_shape_preview_pos[] = {0x0C,0x0E, 0x0D,0x0E, 0x0C,0x0F, 0x0D,0x0F};
uint8_t remove_tiles[360] = {0};

/* FUNCTIONS */
uint8_t get_hard_drop_y();
void set_shape_sprites()
{
    // positions
    move_sprite(4, shape_x*8+8, shape_y*8+16);
    move_sprite(5, shape_x*8+16, shape_y*8+16);
    move_sprite(6, shape_x*8+8, shape_y*8+24);
    move_sprite(7, shape_x*8+16, shape_y*8+24);

    // sprites
    for (uint8_t i = 0; i<4; i++)
    {
        uint8_t n = shape_sprit_num[shape_type*4+i];
        if(n == 8) {
            set_sprite_tile(4+i, 0);
            set_sprite_tile(8+i, 0);
        }
        else
        {
            set_sprite_tile(4+i, SHAPES_SPRITE_START+shape_letters[n]);
            set_sprite_tile(8+i, HD_INDICATOR_START+shape_letters[n]);
        }
    }

    // HD indicator sprite positions
    uint8_t hd_y = get_hard_drop_y();
    move_sprite(8, shape_x*8+8, hd_y*8+16);
    move_sprite(9, shape_x*8+16, hd_y*8+16);
    move_sprite(10, shape_x*8+8, hd_y*8+24);
    move_sprite(11, shape_x*8+16, hd_y*8+24);

    // update on ground flag
    if (hd_y == shape_y) is_on_ground = 1;
    else is_on_ground = 0;
}
uint16_t get_shape_col_neighbors(uint8_t x, uint8_t y)
{
    uint16_t neighbors = 0;
    uint8_t n_tiles[16];
    get_bkg_tiles(x-1, y-1, 4, 4, &n_tiles[0]);

    neighbors = 0
        | (uint16_t)(n_tiles[1] != 1) << 11
        | (uint16_t)(n_tiles[2] != 1) << 10
        | (uint16_t)(n_tiles[4] != 1) << 9
        | (uint16_t)(n_tiles[5] != 1) << 8
        | (uint16_t)(n_tiles[6] != 1) << 7
        | (n_tiles[7] != 1) << 6
        | (n_tiles[8] != 1) << 5
        | (n_tiles[9] != 1) << 4
        | (n_tiles[10] != 1) << 3
        | (n_tiles[11] != 1) << 2
        | (n_tiles[13] != 1) << 1
        | (n_tiles[14] != 1);


    return neighbors;
}

uint8_t get_shape_rot_neighbors()
{
    uint8_t neighbors = 0;
    uint8_t n_tiles[4];
    get_bkg_tiles(shape_x, shape_y, 2, 2, &n_tiles[0]);

    neighbors = (n_tiles[0] != 1) << 3 | (n_tiles[1] != 1) << 2 | (n_tiles[2] != 1) << 1 | (n_tiles[3] != 1);

    return neighbors;
}

uint8_t move_shape(uint8_t dir)
{
    // get all neighbors for collision
    neighbor = get_shape_col_neighbors(shape_x, shape_y);

    if ((neighbor & shape_col_check[shape_type*4+dir]) != 0)
    {
        return 0;
    }

    switch(dir)
    {
        case LEFT:
            shape_x--;
            play_sound(MOVE);
            break;
        case RIGHT:
            shape_x++;
            play_sound(MOVE);
            break;
        case UP:
            shape_y--;
            break;
        case DOWN:
            shape_y++;
            break;
    }

    set_shape_sprites();

    return 1;
}
void rotate_shape(uint8_t d)
{
    uint8_t new_shape_type = 0;

    if(d == 0) new_shape_type = rotate_cw[shape_type];
    else new_shape_type = rotate_ccw[shape_type];
    
    uint8_t neighbors = get_shape_rot_neighbors();

    if ((neighbors & shape_rot_check[new_shape_type]) != 0) return;

    shape_type = new_shape_type;

    set_shape_sprites();
}
void set_next_shape_preview(enum shapes type, uint8_t *bag_pos)
{
    for (uint8_t i = 0; i<4; i++)
    {
        uint8_t n = shape_sprit_num[type*4+i];
        if(n == 8) set_bkg_tile_xy(next_shape_preview_pos[i*2], next_shape_preview_pos[i*2+1], 1);
        else set_bkg_tile_xy(next_shape_preview_pos[i*2], next_shape_preview_pos[i*2+1], UPCOMING_TILES_START + *(bag_pos+n));
    }
}
void write_tiles()
{
    for (uint8_t i = 0; i<4; i++)
    {
        uint8_t n = shape_sprit_num[shape_type*4+i];
        if(n == 8) continue;
        else set_bkg_tile_xy(shape_x + i%2, shape_y + i/2, PLACED_TILES_START+shape_letters[n]);
    }
}
void check_words_hor(uint8_t x, uint8_t y, uint8_t len, uint8_t* remove_tiles)
{
    uint8_t tiles[11];
    get_bkg_tiles(0, y, 11, 1, &tiles[0]);
    uint8_t start_x = 1;
    if(x > len) start_x = x-len+1;
    uint8_t i = 0;

    // find start letter position
    for(start_x; start_x < x+1; start_x++)
    {
        if(tiles[start_x] != 1)
        {
            // are we gonna hit the end? if so: return
            if(start_x+len-1 > 10) return;
            // test if any of the other letters are empty, if so: return
            char word[7];
            uint8_t has_untested = 0;
            for(i=1; i<len+1; i++)
            {
                if(start_x+i-1 > x && tiles[start_x+i-1] == 1) return;
                has_untested = has_untested || (*(remove_tiles + y*20 + start_x+(i-1)) == 0);
                word[i-1] = tiles[start_x + i-1] - PLACED_TILES_START + 97;
            }
            if (!has_untested) return;
            word[len] = 0;
            if(is_word(word, len))
            {
                write_word_to_hist(word, len);
                add_score(len);
                for (uint8_t i = 0; i<len; i++)
                {
                    *(remove_tiles + y * 20 + start_x + i) = 1;
                }
            }
        }
    }
}
void check_words_ver(uint8_t x, uint8_t y, uint8_t len, uint8_t* remove_tiles)
{
    uint8_t tiles[17];
    get_bkg_tiles(x, 0, 1, 17, &tiles[0]);
    uint8_t start_y = 1;
    if(y > len) start_y = y-len+1;
    uint8_t i = 0;

    // find start letter position
    for(start_y; start_y < y+1; start_y++)
    {
        if(tiles[start_y] != 1)
        {
            // are we gonna hit the end? if so: return
            if(start_y+len-1 > 16) return;
            // test if any of the other letters are empty, if so: return
            char word[7];
            uint8_t has_untested = 0;
            for(i=1; i<len+1; i++)
            {
                EMU_printf("%u", tiles[start_y+i-1]);
                if(start_y+i-1 > y && tiles[start_y+i-1] == 1) return;
                has_untested = has_untested || (*(remove_tiles + (start_y+i-1)*20 + x) == 0);
                word[i-1] = tiles[start_y + i-1] - PLACED_TILES_START + 97;
            }
            if (!has_untested) return;
            word[len] = 0;
            if(is_word(word, len))
            {
                write_word_to_hist(word, len);
                add_score(len);
                for (uint8_t i = 0; i<len; i++)
                {
                    *(remove_tiles + (start_y+i) * 20 + x) = 1;
                }
            }
        }
    }
}
void check_words(uint8_t x, uint8_t y, uint8_t* remove_tiles)
{
    check_words_hor(x, y, 6, &remove_tiles[0]);
    check_words_hor(x, y, 5, &remove_tiles[0]);
    check_words_hor(x, y, 4, &remove_tiles[0]);
    check_words_hor(x, y, 3, &remove_tiles[0]);

    check_words_ver(x, y, 6, &remove_tiles[0]);
    check_words_ver(x, y, 5, &remove_tiles[0]);
    check_words_ver(x, y, 4, &remove_tiles[0]);
    check_words_ver(x, y, 3, &remove_tiles[0]);
}
void grab_next_shape(uint8_t do_check)
{
    // write old shape
    write_tiles();

    // lose
    if(shape_y == 1){
        set_gamestate(4);
        return;
    }

    // check for words
    if(do_check) {
        memset(&remove_tiles[0], 0, 360);
        if(shape_sprit_num[shape_type*4]   != 8) check_words(shape_x,   shape_y,   &remove_tiles[0]);
        if(shape_sprit_num[shape_type*4+1] != 8) check_words(shape_x+1, shape_y,   &remove_tiles[0]);
        if(shape_sprit_num[shape_type*4+2] != 8) check_words(shape_x,   shape_y+1, &remove_tiles[0]);
        if(shape_sprit_num[shape_type*4+3] != 8) check_words(shape_x+1, shape_y+1, &remove_tiles[0]);
        update_score();
    }
    
    for(uint16_t i=0; i<360; i++)
    {
        if(remove_tiles[i]) set_bkg_tile_xy(i%20, i/20, 1);
    }

    // update current shape
    uint8_t count = shape_letter_count[next_shape_type];
    memcpy(&shape_letters[0], &bag[0], count);
    shape_type = next_shape_type;
    shape_x = SHAPE_SPAWN_X;
    shape_y = SHAPE_SPAWN_Y;
    set_shape_sprites();

    // remove letters from bag
    shift_bag(&bag[0], count);
    set_upcoming(&bag[0]);

    // randomise new next shape
    next_shape_type = shape_bag[((uint8_t)rand()) % (uint8_t)4];

    // set next shape preview
    set_next_shape_preview(next_shape_type, &bag[0]);

    // reset drop timer
    drop_timer = 0; 
}
uint8_t get_hard_drop_y()
{
    uint8_t hd_y = shape_y;
    for(uint8_t y = shape_y; y < 18; y++)
    {
        neighbor = get_shape_col_neighbors(shape_x, y);

        if ((neighbor & shape_col_check[shape_type*4+DOWN]) == 0)
        {
            hd_y = y+1;
        }
        else
        {
            return hd_y;
        }
    }
    return hd_y;
}