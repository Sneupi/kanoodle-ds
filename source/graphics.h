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

extern Sprite flipIcon;
extern Sprite rotateIcon;
extern Sprite noodleIcon;
extern PolySprite noodleHighlight;
extern PolySprite noodleMain[12];
extern PolySprite noodleSub[12];
extern Sprite noodleOnIcon[12];
extern Sprite noodleOffIcon[12];

Sprite init_sprite(OamState *oam, int *nextId, u16 *gfx, int priority, int x, int y, int w, int h);

PolySprite init_poly_sprite(Polyomino p, OamState *oam, int *nextId, u16 *gfx, int priority, int x, int y, int w, int h);

void init_graphics();
