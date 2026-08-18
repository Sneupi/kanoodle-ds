#pragma once

/**
 * General use libnds sprite library
 */

#include <nds.h>

// sprite graphics template
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

// get next uninitialized sprite id
unsigned int next_oam_id(OamState *oam);

// get next uninitialized palette slot
unsigned int next_pal_id(OamState *oam);

// copies gfx data into vram segments at palette slot [0,15]
SpriteGfx init_sprite_gfx(OamState *oam, SpriteTemplate tem);

// initializes a sprite at OAM slot id [0,127]
Sprite init_sprite(SpriteGfx gfx);

// set sprite location x,y
void set_sprite(Sprite *s, int x, int y);

// shift sprite location by dx,dy
void shift_sprite(Sprite *s, int dx, int dy);

// Fast fixed-point HSV color modifier for BGR555
u16 modify_hsv(u16 color, int hue_shift, int sat_scale, int val_scale);

/**
 * @brief Helper to process an entire palette block
 * @param hue_shift Integer in range 0 to 1535 (e.g., 256 rotates the color wheel by 60°).
 * @param sat_scale 256 maintains current saturation, 0 converts to greyscale, 512 doubles saturation.
 * @param val_scale 256 maintains current brightness, 128 cuts brightness in half, 512 doubles it (clamped to max 31).
 */
void modify_palette_hsv(u16 *src_pal, u16 *dest_pal, int count, int hue_shift, int sat_scale, int val_scale);
