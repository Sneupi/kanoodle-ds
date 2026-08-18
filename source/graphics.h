#pragma once

/**
 * Game graphics for Kanoodle DS
 */

#include "polyomino.h"
#include "sprite.h"

#define POLYBEAD_WIDTH 20 // pixel width
#define POLYBEAD_HEIGHT 20 // pixel height

// polyomino sprite instance (kanoodle noodle)
typedef struct
{
    int x;
    int y;
    Polyomino poly;
    int size;
    Sprite sprites[MAX_POLY_CELLS];
} PolySprite;

// polyomino sprite base coord (x,y) relocation with cell-relative shifts
void set_poly_sprite(PolySprite *p, int x, int y);

// polyomino sprite relative shift of base coords (x,y)
void shift_poly_sprite(PolySprite *p, int dx, int dy);

// initialize a multi-sprite polyomino sprite in OAM
PolySprite init_poly_sprite(Polyomino poly, SpriteGfx gfx);
