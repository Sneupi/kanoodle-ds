#pragma once

#include "polyomino.h"

// vram-initialized sprite instance
typedef struct
{
    int x;
    int y;
    int w;
    int h;
    OamState *oam;
    int oamId;
} Sprite;

// polyomino sprite instance
typedef struct
{
    int x;
    int y;
    Polyomino poly;
    int size;
    Sprite sprites[MAX_POLY_CELLS];
} PolySprite;

extern Sprite flipSprite;
extern Sprite rotateSprite;
extern Sprite panelSprite;
extern PolySprite selectionPolySprite;
extern PolySprite noodlePolySpritesMain[12];
extern PolySprite noodlePolySpritesSub[12];
extern Sprite noodleOffSprites[12];
extern Sprite noodleOnSprites[12];

extern int logoBg;
extern int controlHintsBg;
extern int backgroundBgMain;
extern int difficultyBg;
extern int boardBg;
extern int backgroundBgSub;

/**
 * Game Graphics
 */

void init_graphics();

void menu_screen();

void game_screen();

void hide_panel(bool hide);

void press_panel_button(int i);

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