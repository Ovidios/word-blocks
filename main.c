#define DAS_DELAY_FIRST 16
#define DAS_DELAY_ADD 2
#define SOFT_DROP_DIV 7
#define WORDS_PER_LEVEL 6
#define SLIDE_FRAMES 60

#include <gb/gb.h>
#include <gb/cgb.h>
#include <gb/emu_debug.h>
#include <stdio.h>
#include "bgmap.h"
#include "bgtiles.h"
#include "winmap.h"
#include "wintiles.h"
#include "menu.h"
#include "sprites.h"
#include "sounds.c"
#include "bag.c"
#include "history.c"
#include "shape.c"
#include "palettes.c"

enum gamestates {PLAY, PAUSE, GAMEOVER, MENU, TRANSITION};

void init();
uint8_t checkInput(uint8_t last_input);
void updateSwitches();
void update_level();
uint8_t das_right = DAS_DELAY_FIRST;
uint8_t das_left = DAS_DELAY_FIRST;
uint32_t score = 0;
uint8_t drop_bonus = 0;
uint8_t words_till_level = WORDS_PER_LEVEL;
enum gamestates gamestate = PLAY;
char* over_text = "gameoverz{";
char* pause_text = "gamepaused";

// lookup table for sin -- floor(sin(x*2/32*pi)*2)
int8_t sin_times_2[] = {0,0,0,1,1,1,1,1,2,1,1,1,1,1,0,0,0,-1,-1,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-1,-1};

extern uint8_t is_word(unsigned char* word, uint8_t len);

enum shapes shape = BLOCK_1;

uint8_t level = 0;
uint8_t start_level = 0;
uint8_t frame_n = 0;
uint8_t do_soft_drop = 0;
uint8_t drop_timer = 0;
uint16_t transition_timer = 0;
uint8_t menu_cursor_y = 0;
uint8_t music = 1;
uint8_t sound = 1;
const uint8_t frames_per_level[] = {
    53, 49, 45, 41, 37, 33, 28, 22, 17, 11, 10, 9, 8, 7, 6, 6, 5, 5, 4, 4, 3 // these are the GB tetris ones
};
const uint16_t score_per_len[] = {0,0,0, 40, 100, 300, 1200};

/* 
 ~ SPRITES ~
   0-3      offset upcoming letters
   4-7      current shape letters
   8-11     hard drop indicator
   12-13    level indicator
   14-24    pause text
   25       menu cursor
*/

void main() {
	init();
    if(_cpu == CGB_TYPE) cpu_fast();

    uint8_t last_input = 0;

    updateSwitches();

    set_gamestate(MENU);

    // main loop
	while(1) {
        frame_n++;
        if(gamestate == PLAY)
        {
            drop_timer++;
        }
        last_input = checkInput(last_input);
        
        // transition
        if(gamestate == TRANSITION)
        {
            VBK_REG = 1;
            set_bkg_tile_xy(transition_timer%20, 2*(transition_timer/20), 3);
            set_bkg_tile_xy(19-(transition_timer%20), 19-(2*(transition_timer/20)), 3);

            VBK_REG = 0;
            set_bkg_tile_xy(transition_timer%20, 2*(transition_timer/20), 0);
            set_bkg_tile_xy(19-(transition_timer%20), 19-(2*(transition_timer/20)), 0);

            transition_timer++;

            if(transition_timer > 200)
            {
                waitpad(J_START | J_SELECT | J_A | J_B);
                set_gamestate(GAMEOVER);
            }
        }
        // menu
        if(gamestate == MENU)
        {
            move_sprite(25, 42 + sin_times_2[frame_n%32], 88 + menu_cursor_y * 16);
            set_win_tile_xy(12, 9, 56+(start_level/10));
            set_win_tile_xy(13, 9, 56+(start_level%10));

            set_win_tile_xy(12, 15, 56+(curr_pal/10));
            set_win_tile_xy(13, 15, 56+(curr_pal%10));
        }

        // handle falling
        uint8_t frames = frames_per_level[level];
        if (level > 20) frames = 3;
        if (is_on_ground) frames += SLIDE_FRAMES;
        if (do_soft_drop) frames = frames/SOFT_DROP_DIV+1;
        if(drop_timer >= frames)
        {
            // drop
            if (!move_shape(DOWN))
            {
                play_sound(DROP);
                grab_next_shape(1);
            }
            if (do_soft_drop) drop_bonus++;
            drop_timer = 0;
        }

        if (gamestate == PAUSE || gamestate == GAMEOVER) update_pause_text();

        wait_vbl_done();
	}
}

void init() {
    switch_palette();

    SWITCH_ROM(9);

    // set background, window & sprite tileset
    set_bkg_data(0, 188, BGTiles);
    set_sprite_data(1, 90, Sprites);

    // set sprites initially
    move_sprite(0, 132, 136);
    move_sprite(1, 140, 136);
    move_sprite(2, 148, 136);
    move_sprite(3, 156, 136);

    set_sprite_prop(4, 1);
    set_sprite_prop(5, 1);
    set_sprite_prop(6, 1);
    set_sprite_prop(7, 1);

    set_sprite_prop(8, 2);
    set_sprite_prop(9, 2);
    set_sprite_prop(10, 2);
    set_sprite_prop(11, 2);

    move_sprite(12, 12, 20);
    move_sprite(13, 20, 20);
    set_sprite_prop(12, 3);
    set_sprite_prop(13, 3);

    // set bg & win map palettes for each tile
    VBK_REG = 1;        // switch to 2nd video memory bank
    set_bkg_tiles(0, 0, BGMapWidth, BGMapHeight, BGMapPLN1);
    set_win_tiles(0, 0, BGMapWidth, BGMapHeight, WinMapPLN1);

    // set bg & win map tiles
    VBK_REG = 0;
    set_bkg_tiles(0, 0, BGMapWidth, BGMapHeight, BGMapPLN0);
    set_win_tiles(0, 0, BGMapWidth, BGMapHeight, WinMapPLN0);

	DISPLAY_ON;		    // Turn on the display

    // stuff
    draw_bag(bag, new_bag);
    draw_bag(bag, new_bag);
    set_upcoming(&bag[0]);
    update_score();
    update_level();
    grab_next_shape(0);
}

void updateSwitches() {
    HIDE_WIN;
    SHOW_SPRITES;
    SHOW_BKG;
}

void add_score(uint8_t len)
{
    score += score_per_len[len] * (level + 1) + drop_bonus;
    drop_bonus = 0;
    words_till_level--;
    if (words_till_level == 0)
    {
        words_till_level = WORDS_PER_LEVEL;
        level++;
        update_level();
    }
}

void update_score()
{
    set_bkg_tile_xy(12, 1, 152 + score/1000000);
    set_bkg_tile_xy(13, 1, 152 + (score%1000000)/100000);
    set_bkg_tile_xy(14, 1, 152 + (score%100000)/10000);
    set_bkg_tile_xy(15, 1, 152 + (score%10000)/1000);
    set_bkg_tile_xy(16, 1, 152 + (score%1000)/100);
    set_bkg_tile_xy(17, 1, 152 + (score%100)/10);
    set_bkg_tile_xy(18, 1, 152 + score%10);
}

void update_pause_score()
{
    set_win_tile_xy(11, 9, 56 + score/1000000);
    set_win_tile_xy(12, 9, 56 + (score%1000000)/100000);
    set_win_tile_xy(13, 9, 56 + (score%100000)/10000);
    set_win_tile_xy(14, 9, 56 + (score%10000)/1000);
    set_win_tile_xy(15, 9, 56 + (score%1000)/100);
    set_win_tile_xy(16, 9, 56 + (score%100)/10);
    set_win_tile_xy(17, 9, 56 + score%10);
}

void update_level()
{
    set_sprite_tile(12, 53+level/10);
    set_sprite_tile(13, 53+level%10);
}

void update_pause_text()
{
    for (uint8_t i=0; i<4; i++)
    {
        move_sprite(14+i, 72+i*8, 48 + sin_times_2[(frame_n+i)%32]);
    }
    for (uint8_t i=0; i<6; i++)
    {
        move_sprite(18+i, 64+i*8, 58 + sin_times_2[(frame_n+4+i)%32]);
    }
}
void set_pause_text()
{
    // set text
    for (uint8_t i=0; i<10; i++)
    {
        if(gamestate == PAUSE) set_sprite_tile(14+i, pause_text[i]-97+63);
        else if(gamestate == GAMEOVER) set_sprite_tile(14+i, over_text[i]-97+63);
        else set_sprite_tile(14+i, 0);
    }
}

void pause()
{
    if (gamestate != PLAY && gamestate != PAUSE) return;
    if(gamestate == PLAY)
    {
        gamestate = PAUSE;
        // tiles
        SWITCH_ROM(9);
        set_win_data(0, 154, WinTiles);
        update_pause_score();
        // set bg palettes for each tile
        VBK_REG = 1;        // switch to 2nd video memory bank
        set_win_tiles(0, 0, BGMapWidth, BGMapHeight, WinMapPLN1);
        // set bg & win map tiles
        VBK_REG = 0;
        set_win_tiles(0, 0, BGMapWidth, BGMapHeight, WinMapPLN0);
        SHOW_WIN;
        // level indicator
        move_sprite(12, 60, 77);
        move_sprite(13, 68, 77);
        set_sprite_prop(12, 4);
        set_sprite_prop(13, 4);
        // other sprites
        set_sprite_tile(0, 0);
        set_sprite_tile(1, 0);
        set_sprite_tile(2, 0);
        set_sprite_tile(3, 0);
        set_sprite_tile(4, 0);
        set_sprite_tile(5, 0);
        set_sprite_tile(6, 0);
        set_sprite_tile(7, 0);
        set_sprite_tile(8, 0);
        set_sprite_tile(9, 0);
        set_sprite_tile(10, 0);
        set_sprite_tile(11, 0);
        // pause text
        set_sprite_tile(14, 0);
        set_sprite_tile(15, 0);
        set_sprite_tile(16, 0);
        set_sprite_tile(17, 0);
        set_sprite_tile(18, 0);
        set_sprite_tile(19, 0);
        set_sprite_tile(20, 0);
        set_sprite_tile(21, 0);
        set_sprite_tile(22, 0);
        set_sprite_tile(23, 0);
        set_sprite_tile(24, 0);
    }
    else
    {
        gamestate = PLAY;
        // tiles
        HIDE_WIN;
        SWITCH_ROM(9);
        set_bkg_data(0, 188, BGTiles);
        SHOW_BKG;
        // level indicator
        move_sprite(12, 12, 20);
        move_sprite(13, 20, 20);
        set_sprite_prop(12, 3);
        set_sprite_prop(13, 3);
    }
    set_pause_text();
}

void set_gamestate(enum gamestates state)
{
    SHOW_SPRITES;
    if(state == gamestate) return;
    if ((state == PAUSE || state == PLAY) && (gamestate == PAUSE || gamestate == PLAY))
    {
        pause(); return;
    }
    if (state == GAMEOVER)
    {
        gamestate = state;
        // tiles
        SWITCH_ROM(9);
        set_win_data(0, 154, WinTiles);
        // set bg palettes for each tile
        VBK_REG = 1;        // switch to 2nd video memory bank
        set_win_tiles(0, 0, BGMapWidth, BGMapHeight, WinMapPLN1);
        // set bg & win map tiles
        VBK_REG = 0;
        set_win_tiles(0, 0, BGMapWidth, BGMapHeight, WinMapPLN0);
        update_pause_score();
        SHOW_WIN;
        // level indicator
        move_sprite(12, 60, 77);
        move_sprite(13, 68, 77);
        set_sprite_prop(12, 4);
        set_sprite_prop(13, 4);
        // other sprites
        set_sprite_tile(0, 0);
        set_sprite_tile(1, 0);
        set_sprite_tile(2, 0);
        set_sprite_tile(3, 0);
        set_sprite_tile(4, 0);
        set_sprite_tile(5, 0);
        set_sprite_tile(6, 0);
        set_sprite_tile(7, 0);
        set_sprite_tile(8, 0);
        set_sprite_tile(9, 0);
        set_sprite_tile(10, 0);
        set_sprite_tile(11, 0);
        set_sprite_tile(25, 0);
        // text
        set_pause_text();
        return;
    }
    if (state == TRANSITION)
    {
        HIDE_SPRITES;
        transition_timer = 0;
        gamestate = state;
        return;
    }
    if (state == PLAY)
    {
        gamestate = PLAY;
        // tiles
        HIDE_WIN;
        SWITCH_ROM(9);
        set_bkg_data(0, 188, BGTiles);
        // set bg map palettes & tiles
        VBK_REG = 1;        // switch to 2nd video memory bank
        set_bkg_tiles(0, 0, BGMapWidth, BGMapHeight, BGMapPLN1);
        VBK_REG = 0;
        set_bkg_tiles(0, 0, BGMapWidth, BGMapHeight, BGMapPLN0);
        SHOW_BKG;
        // level indicator
        move_sprite(12, 12, 20);
        move_sprite(13, 20, 20);
        set_sprite_prop(12, 3);
        set_sprite_prop(13, 3);
        // pause text
        set_sprite_tile(14, 0);
        set_sprite_tile(15, 0);
        set_sprite_tile(16, 0);
        set_sprite_tile(17, 0);
        set_sprite_tile(18, 0);
        set_sprite_tile(19, 0);
        set_sprite_tile(20, 0);
        set_sprite_tile(21, 0);
        set_sprite_tile(22, 0);
        set_sprite_tile(23, 0);
        set_sprite_tile(24, 0);
        set_sprite_tile(25, 0);
        return;
    }
    if (state == MENU)
    {
        gamestate = state;
        // tiles
        SWITCH_ROM(9);
        set_win_data(0, 154, WinTiles);
        // set bg palettes for each tile
        VBK_REG = 1;        // switch to 2nd video memory bank
        set_win_tiles(0, 0, BGMapWidth, BGMapHeight, MenuMapPLN1);
        // set bg & win map tiles
        VBK_REG = 0;
        set_win_tiles(0, 0, BGMapWidth, BGMapHeight, MenuMapPLN0);
        
        set_win_tile_xy(13, 11, 119*music+1);
        set_win_tile_xy(13, 13, 119*sound+1);
        SHOW_WIN;
        // pause text
        set_sprite_tile(0, 0);
        set_sprite_tile(1, 0);
        set_sprite_tile(2, 0);
        set_sprite_tile(3, 0);
        set_sprite_tile(4, 0);
        set_sprite_tile(5, 0);
        set_sprite_tile(6, 0);
        set_sprite_tile(7, 0);
        set_sprite_tile(8, 0);
        set_sprite_tile(9, 0);
        set_sprite_tile(10, 0);
        set_sprite_tile(11, 0);
        set_sprite_tile(12, 0);
        set_sprite_tile(13, 0);
        set_sprite_tile(14, 0);
        set_sprite_tile(15, 0);
        set_sprite_tile(16, 0);
        set_sprite_tile(17, 0);
        set_sprite_tile(18, 0);
        set_sprite_tile(19, 0);
        set_sprite_tile(20, 0);
        set_sprite_tile(21, 0);
        set_sprite_tile(22, 0);
        set_sprite_tile(23, 0);
        set_sprite_tile(24, 0);
        set_sprite_tile(25, 90);
        set_sprite_prop(25, 4);
    }
}
void reset_game()
{
    uint16_t seed = LY_REG;
    seed |= (uint16_t)DIV_REG << 8;
    initrand(seed);
    tiles_in_bag = 0;
    draw_bag(&bag[0], &new_bag[0]);
    grab_next_shape(0);
    score = 0;
    frame_n = 0;
    drop_timer = 0;
    level = start_level;
    words_till_level = WORDS_PER_LEVEL;
    drop_bonus;
    set_gamestate(PLAY);
    set_upcoming(&bag[0]);
    set_next_shape_preview(next_shape_type, &bag[0]);
    update_score();
    update_level();
    set_shape_sprites();
}

uint8_t checkInput(uint8_t last_joypad_state) {
    uint8_t joypad_state = joypad();

    /* DEAL WITH DAS */
    if (joypad_state & J_RIGHT && gamestate == PLAY) {
        das_right--;
        if(das_right == 255) das_right = DAS_DELAY_ADD;
    }
    else das_right = DAS_DELAY_FIRST;

    if (joypad_state & J_LEFT  && gamestate == PLAY) {
        das_left--;
        if(das_left == 255) das_left = DAS_DELAY_ADD;
    }
    else das_left = DAS_DELAY_FIRST;

    /* ACTUAL MOVEMENT */
    if (joypad_state & J_A && !(last_joypad_state & J_A)  && gamestate == PLAY) {
        rotate_shape(0);
    }

    if (joypad_state & J_B && !(last_joypad_state & J_B)  && gamestate == PLAY) {
        rotate_shape(1);
    }

    if (joypad_state & J_A && !(last_joypad_state & J_A) && gamestate == MENU)
    {
        if (menu_cursor_y == 0) reset_game();
        if (menu_cursor_y == 1)
        {
            music = !music;
            set_win_tile_xy(13, 11, 119*music+1);
        }
        if (menu_cursor_y == 2)
        {
            sound = !sound;
            set_win_tile_xy(13, 13, 119*sound+1);
        }
    }

    if (joypad_state & J_START && !(last_joypad_state & J_START)) {
        if (gamestate == PAUSE || gamestate == PLAY) pause();
        else if (gamestate == GAMEOVER) set_gamestate(MENU);
    }

    if (joypad_state & J_SELECT && !(last_joypad_state & J_SELECT) && gamestate == PLAY) {
        set_gamestate(TRANSITION);
    }

    if (joypad_state & J_RIGHT && (!(last_joypad_state & J_RIGHT) || das_right == 0) && gamestate == PLAY)
    {
        move_shape(RIGHT);
    }

    if (joypad_state & J_LEFT && (!(last_joypad_state & J_LEFT) || das_left == 0) && gamestate == PLAY)
    {
        move_shape(LEFT);
    }

    if (joypad_state & J_DOWN  && gamestate == PLAY) do_soft_drop = 1;
    else do_soft_drop = 0;

    if (joypad_state & J_DOWN && (!(last_joypad_state & J_DOWN)) && gamestate == MENU)
    {
        menu_cursor_y++;
        if(menu_cursor_y == 4) menu_cursor_y = 0;
    }
    if (joypad_state & J_UP && (!(last_joypad_state & J_UP)) && gamestate == MENU)
    {
        menu_cursor_y--;
        if(menu_cursor_y == 255) menu_cursor_y = 3;
    }
    if (joypad_state & J_LEFT && (!(last_joypad_state & J_LEFT)) && gamestate == MENU)
    {
        if(menu_cursor_y == 0)
        {
            start_level--;
            if(start_level == 255) start_level = 20;
        }
        if(menu_cursor_y == 3)
        {
            curr_pal--;
            if(curr_pal == 255) curr_pal = MAX_PAL;
            switch_palette();
        }
    }
    if (joypad_state & J_RIGHT && (!(last_joypad_state & J_RIGHT)) && gamestate == MENU)
    {
        if(menu_cursor_y == 0)
        {
            start_level++;
            if(start_level == 21) start_level = 0;
        }
        if(menu_cursor_y == 3)
        {
            curr_pal++;
            if(curr_pal == MAX_PAL+1) curr_pal = 0;
            switch_palette();
        }
    }

    if (joypad_state & J_UP && !(last_joypad_state & J_UP) && gamestate == PLAY)
    {
        shape_y = get_hard_drop_y();
        play_sound(DROP);
        set_shape_sprites();
        grab_next_shape(1);
        drop_bonus += 16;
    }

    return joypad_state;
}