#pragma once

#include <nds.h>
#include "polyomino.h" // for PolySprite

static unsigned int nextOamIdMain = 0;
static unsigned int nextOamIdSub = 0;
static unsigned int nextPalSlotMain = 0;
static unsigned int nextPalSlotSub = 0;

// sprite graphics tem
typedef struct
{
    SpriteSize size;
    SpriteColorFormat colorFormat;
    const unsigned int *tiles;
    const unsigned short *pal;
    size_t tilesLen;
    size_t palLen;

} SpriteTemplate;

// vram-initialized sprite graphics
typedef struct
{
    SpriteTemplate tem;
    OamState *oam;
    u16 *gfx;
    int palSlot;

} SpriteGfx;

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

unsigned int nextOamId(OamState *oam)
{
    return (oam == &oamMain) ? nextOamIdMain++ : nextOamIdSub++;
}

unsigned int nextPalSlot(OamState *oam)
{
    return (oam == &oamMain) ? nextPalSlotMain++ : nextPalSlotSub++;
}

// copies gfx data into vram segments at palette slot [0,15]
SpriteGfx init_sprite_gfx(OamState *oam, SpriteTemplate tem)
{
    // allocate vram segment
    u16 *gfx = oamAllocateGfx(oam, tem.size, tem.colorFormat);

    // load with tiles
    dmaCopy(tem.tiles, gfx, tem.tilesLen);

    // calculate palette slot address & load pal
    unsigned int palSlot = nextPalSlot(oam);
    void *palDest = ((oam == &oamMain) ? SPRITE_PALETTE : SPRITE_PALETTE_SUB) + (palSlot * 16);
    dmaCopy(tem.pal, palDest, tem.palLen);

    return (SpriteGfx){.tem = tem, .oam = oam, .gfx = gfx, .palSlot = palSlot};
}

// initializes a sprite at OAM slot id [0,127]
Sprite init_sprite(SpriteGfx gfx)
{
    int id = nextOamId(gfx.oam);
    oamSet(
        gfx.oam,             // OAM engine
        id,                  // OAM ID (0 to 127)
        0, 0,                // X, Y position
        0,                   // Priority (0 = top)
        gfx.palSlot,         // Palette index (0 for first 16-color bank)
        gfx.tem.size,        // Size
        gfx.tem.colorFormat, // Color format
        gfx.gfx,             // Pointer to VRAM tile data
        -1,                  // Rotation/Scale index (-1 = none)
        false,               // Double size flag
        false,               // Hide sprite
        false, false,        // VFlip, HFlip
        false                // Mosaic
    );
    return (Sprite){.x = 0, .y = 0, .w = 32, .h = 32, .oam = gfx.oam, .oamId = id}; // FIXME cheap hack make all 32x32
}

void set_sprite(Sprite *s, int x, int y)
{
    s->x = x;
    s->y = y;
    oamSetXY(s->oam, s->oamId, x, y);
}

void shift_sprite(Sprite *s, int dx, int dy)
{
    set_sprite(s, (s->x + dx), (s->y + dy));
}

typedef struct
{
    int x;
    int y;
    Polyomino poly;
    int size;
    Sprite sprites[MAX_POLY_CELLS];
} PolySprite;

void set_poly_sprite(PolySprite *p, int x, int y)
{
    p->x = x;
    p->y = y;
    for (int i = 0; i < p->size; i++)
    {
        Sprite *s = &(p->sprites[i]);
        int sprite_offset_x = x + (s->w * p->poly.cell[i].x);
        int sprite_offset_y = y + (s->h * p->poly.cell[i].y);
        set_sprite(s, sprite_offset_x, sprite_offset_y);
    }
}

void shift_poly_sprite(PolySprite *p, int dx, int dy)
{
    set_poly_sprite(p, (p->x + dx), (p->y + dy));
}

PolySprite init_poly_sprite(Polyomino poly, SpriteGfx gfx)
{
    PolySprite p = {0};
    p.poly = zero_bounded(poly);
    p.size = poly.size;
    p.x = 0;
    p.y = 0;

    for (int i = 0; i < p.size; i++) {
        Sprite *s = &p.sprites[i];
        *s = init_sprite(gfx);
        // FIXME: hackey way to get polyBead gfx to align pixel perfect
        s->h = 20;
        s->w = 20;
    }

    set_poly_sprite(&p, 0, 0);
    return p;
}