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
extern Sprite noodleOnSprites[12];
extern Sprite noodleOffSprites[12];

extern int logoBg;
extern int controlHintsBg;
extern int backgroundBgMain;
extern int difficultyBg;
extern int boardBg;
extern int backgroundBgSub;

Sprite init_sprite(OamState *oam, int *nextId, u16 *gfx, int priority, int x, int y, int w, int h);

PolySprite init_poly_sprite(Polyomino p, OamState *oam, int *nextId, u16 *gfx, int priority, int x, int y, int w, int h);

void init_graphics();
