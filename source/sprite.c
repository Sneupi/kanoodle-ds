
#include "sprite.h"

// universal counters to avoid manual tracking
static unsigned int nextOamIdMain = 0;
static unsigned int nextOamIdSub = 0;
static unsigned int nextPalSlotMain = 0;
static unsigned int nextPalSlotSub = 0;

unsigned int next_oam_id(OamState *oam)
{
    return (oam == &oamMain) ? nextOamIdMain++ : nextOamIdSub++;
}

unsigned int next_pal_id(OamState *oam)
{
    return (oam == &oamMain) ? nextPalSlotMain++ : nextPalSlotSub++;
}

SpriteGfx init_sprite_gfx(OamState *oam, SpriteTemplate tem)
{
    // allocate vram segment
    u16 *gfx = oamAllocateGfx(oam, tem.size, tem.colorFormat);

    // load with tiles
    dmaCopy(tem.tiles, gfx, tem.tilesLen);

    // calculate palette slot address & load pal
    unsigned int palSlot = next_pal_id(oam);
    void *palDest = ((oam == &oamMain) ? SPRITE_PALETTE : SPRITE_PALETTE_SUB) + (palSlot * 16);
    dmaCopy(tem.pal, palDest, tem.palLen);

    return (SpriteGfx){.tem = tem, .oam = oam, .gfx = gfx, .palSlot = palSlot};
}

Sprite init_sprite(SpriteGfx gfx)
{
    int id = next_oam_id(gfx.oam);
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
    return (Sprite){.x = 0, .y = 0, .w = 32, .h = 32, .oam = gfx.oam, .oamId = id}; // FIXME cheap hack that makes all 32x32, replace with drop-in conversions
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

u16 modify_hsv(u16 color, int hue_shift, int sat_scale, int val_scale)
{
    // Extract 5-bit BGR components (0-31 range)
    int r = (color) & 0x1F;
    int g = (color >> 5) & 0x1F;
    int b = (color >> 10) & 0x1F;

    // Preserve the highest bit (used for transparency in sprite palettes)
    u16 alpha_bit = color & 0x8000;

    // Find Min, Max, and Chrominance (Delta)
    int max = (r > g) ? ((r > b) ? r : b) : ((g > b) ? g : b);
    int min = (r < g) ? ((r < b) ? r : b) : ((g < b) ? g : b);
    int delta = max - min;

    // Convert RGB -> HSV
    int h = 0;
    int s = (max == 0) ? 0 : (delta * 256) / max;
    int v = max;

    if (delta > 0)
    {
        if (max == r)
        {
            h = ((g - b) * 256) / delta;
            if (h < 0)
                h += 1536; // 6 sectors * 256
        }
        else if (max == g)
        {
            h = ((b - r) * 256) / delta + 512;
        }
        else
        { // max == b
            h = ((r - g) * 256) / delta + 1024;
        }
    }

    // Apply Adjustments
    h = (h + hue_shift) % 1536;
    if (h < 0)
        h += 1536;

    s = (s * sat_scale) >> 8; // Scale saturation (256 = 100%)
    if (s > 256)
        s = 256;
    if (s < 0)
        s = 0;

    v = (v * val_scale) >> 8; // Scale brightness (256 = 100%)
    if (v > 31)
        v = 31;
    if (v < 0)
        v = 0;

    // Convert HSV -> RGB
    if (s == 0)
    {
        // Achromatic / Greyscale fast path
        return alpha_bit | RGB15(v, v, v);
    }

    int region = h / 256;
    int remainder = h % 256;

    int p = (v * (256 - s)) >> 8;
    int q = (v * (256 - ((s * remainder) >> 8))) >> 8;
    int t = (v * (256 - ((s * (256 - remainder)) >> 8))) >> 8;

    int nr = 0, ng = 0, nb = 0;

    switch (region)
    {
    case 0:
        nr = v;
        ng = t;
        nb = p;
        break;
    case 1:
        nr = q;
        ng = v;
        nb = p;
        break;
    case 2:
        nr = p;
        ng = v;
        nb = t;
        break;
    case 3:
        nr = p;
        ng = q;
        nb = v;
        break;
    case 4:
        nr = t;
        ng = p;
        nb = v;
        break;
    default:
        nr = v;
        ng = p;
        nb = q;
        break;
    }

    return alpha_bit | RGB15(nr, ng, nb);
}

void modify_palette_hsv(u16 *src_pal, u16 *dest_pal, int count, int hue_shift, int sat_scale, int val_scale)
{
    for (int i = 0; i < count; i++)
    {
        dest_pal[i] = modify_hsv(src_pal[i], hue_shift, sat_scale, val_scale);
    }
}
