#pragma once

#include "polyomino.h"

// vram-initialized sprite instance
typedef struct Sprite
{
    int x;                             // 0-256
    int y;                             // 0-192
    int w;                             // true pixel width
    int h;                             // true pixel height
    OamState *oam;                     // &oamSub OR &oamMain
    int oamId;                         // 0-127
    int polyId;                        // on-touch cb param (-1 if N/A)
    void (*callback)(struct Sprite *); // on-touch event
    bool hidden;                       // 0 (show), 1 (hide)
} Sprite;

// polyomino sprite instance
typedef struct
{
    int x;                           // base x coord
    int y;                           // base y coord
    Polyomino poly;                  // polyomino rep
    Sprite *sprites[MAX_POLY_CELLS]; // polyomino cell sprites
} PolySprite;

/**
 * Game Graphics
 */

void init_graphics();

void menu_screen();

void game_screen();

void cb_toggle_panel(Sprite * /*unused*/);

void cb_toggle_noodle(Sprite *s);

void cb_highlight_draggable(Sprite *s);

void cb_flip_highlighted(Sprite * /*unused*/);

void cb_rotate_highlighted(Sprite * /*unused*/);

void handle_input();

/**
 * Sprites
 */

void hide_sprite(Sprite *s, bool hide);

void place_sprite(Sprite *s, int x, int y);

void shift_sprite(Sprite *s, int dx, int dy);

/**
 * PolySprites
 */

void hide_poly_sprite(PolySprite *s, bool hide);

void place_poly_sprite(PolySprite *s, int x, int y);

void shift_poly_sprite(PolySprite *s, int dx, int dy);

void rotate_poly_sprite(PolySprite *s);

void flip_poly_sprite(PolySprite *s);

/**
 * Backgrounds
 */

void hide_bg(int id, bool hide);