
#include <nds.h>
#include "graphics.h"
#include "kanoodle.h"

#include "sprites.h" // auto-gen assets via grit

Sprite flipIcon;
Sprite rotateIcon;
Sprite noodleIcon;
PolySprite noodleHighlight;
PolySprite noodleMain[12];
PolySprite noodleSub[12];
Sprite noodleOnIcon[12];
Sprite noodleOffIcon[12];

Sprite init_sprite(OamState *oam, int *nextId, u16 *gfx, int priority, int x, int y, int w, int h)
{
    oamSet(
        oam,                        // Main or Sub OAM
        *nextId,                    // OAM entry ID (0 to 127)
        x, y,                       // Screen coordinates
        priority,                   // Priority (0-3)
        0,                          // Palette index (0 for 256-color)
        SpriteSize_32x32,           // Sprite dimensions
        SpriteColorFormat_256Color, // Color depth
        gfx,                        // Graphics pointer in VRAM
        -1,                         // Affine index (-1 = no rotation/scaling)
        false,                      // Double size flag
        false,                      // Hide sprite flag
        false, false,               // VFlip, HFlip
        false                       // Mosaic
    );
    return (Sprite){x, y, w, h, oam, (*nextId)++};
}

PolySprite init_poly_sprite(Polyomino p, OamState *oam, int *nextId, u16 *gfx, int priority, int x, int y, int w, int h)
{
    PolySprite ps;
    ps.poly = zero_bounded(p);
    ps.size = p.size;
    ps.x = x;
    ps.y = y;
    for (int i = 0; i < ps.size; i++)
    {
        int xOffset = x + (w * ps.poly.cell[i].x);
        int yOffset = y + (h * ps.poly.cell[i].y);
        ps.sprites[i] = init_sprite(oam, nextId, gfx, priority, xOffset, yOffset, w, h);
    }
    return ps;
}

void init_graphics()
{

    // 1. Initialize Video & VRAM
    videoSetMode(MODE_0_2D);
    videoSetModeSub(MODE_0_2D);

    vramSetBankA(VRAM_A_MAIN_BG);
    vramSetBankB(VRAM_B_MAIN_SPRITE);
    vramSetBankC(VRAM_C_SUB_BG);
    vramSetBankD(VRAM_D_SUB_SPRITE);

    bgExtPaletteEnable();
    bgExtPaletteEnableSub();

    // 2. Initialize OAM with 1D_128 Mapping
    oamInit(&oamMain, SpriteMapping_1D_128, false);
    oamInit(&oamSub, SpriteMapping_1D_128, false);

    // 3. Copy Palettes
    dmaCopy(spritesPal, SPRITE_PALETTE, spritesPalLen);
    dmaCopy(spritesPal, SPRITE_PALETTE_SUB, spritesPalLen);

    // 4. Allocate & Init assets
    // 4.1. Allocate VRAM memory for all 40 frames
    u16 *gfxMain[40];
    u16 *gfxSub[40];

    int spriteSizeInBytes = 32 * 32; // 1024 bytes for 8-bit 32x32
    for (int i = 0; i < 40; i++)
    {
        gfxMain[i] = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_256Color);
        gfxSub[i] = oamAllocateGfx(&oamSub, SpriteSize_32x32, SpriteColorFormat_256Color);

        // Copy the specific frame slice from the sprite sheet array
        u8 *frameOffset = (u8 *)spritesTiles + (i * spriteSizeInBytes);
        dmaCopy(frameOffset, gfxMain[i], spriteSizeInBytes);
        dmaCopy(frameOffset, gfxSub[i], spriteSizeInBytes);
    }

    // 4.2. Set OAM entry attributes w proper
    int nextIdMain = 0;
    int nextIdSub = 0;
    flipIcon = init_sprite(&oamSub, &nextIdSub, gfxSub[0], 3, 224, 160, 32, 32);
    rotateIcon = init_sprite(&oamSub, &nextIdSub, gfxSub[1], 3, 224, 128, 32, 32);
    noodleIcon = init_sprite(&oamSub, &nextIdSub, gfxSub[2], 3, 192, 160, 32, 32);
    noodleHighlight = init_poly_sprite(NOODLE('B'), &oamSub, &nextIdSub, gfxSub[3], 3, 0, 0, 22, 22);
    for (int i = 0; i < 12; i++)
    {
        noodleMain[i] = init_poly_sprite(NOODLES[i], &oamMain, &nextIdMain, gfxMain[4 + i], 0, 100, 100, 20, 20);
        noodleSub[i] = init_poly_sprite(NOODLES[i], &oamSub, &nextIdSub, gfxSub[4 + i], 0, 100, 100, 20, 20);
        noodleOnIcon[i] = init_sprite(&oamSub, &nextIdSub, gfxSub[16 + i], 3, ((i % 6) * 32), (((i / 6) * 32) + 128), 32, 32);
        noodleOffIcon[i] = init_sprite(&oamSub, &nextIdSub, gfxSub[28 + i], 3, ((i % 6) * 32), (((i / 6) * 32) + 128), 32, 32);
    }

    // 4.3. Update hardware OAM
    oamUpdate(&oamMain);
    oamUpdate(&oamSub);
}