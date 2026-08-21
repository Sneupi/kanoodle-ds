
#include <nds.h>
#include "graphics.h"
#include "kanoodle.h"

// auto-gen assets via grit
#include "sprites.h"
#include "logo.h"
#include "controlHints.h"
#include "background.h"
#include "difficulty.h"
#include "board.h"

Sprite flipIcon;
Sprite rotateIcon;
Sprite noodleIcon;
PolySprite noodleHighlight;
PolySprite noodleMain[12];
PolySprite noodleSub[12];
Sprite noodleOnIcon[12];
Sprite noodleOffIcon[12];

int logoBg;
int controlHintsBg;
int backgroundBgMain;
int difficultyBg;
int boardBg;
int backgroundBgSub;

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
    for (int i = 0; i < 12; i++)
        noodleMain[i] = init_poly_sprite(NOODLES[i], &oamMain, &nextIdMain, gfxMain[4 + i], 0, 100, 100, 20, 20);
    for (int i = 0; i < 12; i++)
        noodleSub[i] = init_poly_sprite(NOODLES[i], &oamSub, &nextIdSub, gfxSub[4 + i], 0, 100, 100, 20, 20);
    noodleHighlight = init_poly_sprite(NOODLE('B'), &oamSub, &nextIdSub, gfxSub[3], 0, 0, 0, 22, 22);
    flipIcon = init_sprite(&oamSub, &nextIdSub, gfxSub[0], 0, 224, 160, 32, 32);
    rotateIcon = init_sprite(&oamSub, &nextIdSub, gfxSub[1], 0, 224, 128, 32, 32);
    noodleIcon = init_sprite(&oamSub, &nextIdSub, gfxSub[2], 0, 192, 160, 32, 32);
    for (int i = 0; i < 12; i++)
        noodleOnIcon[i] = init_sprite(&oamSub, &nextIdSub, gfxSub[16 + i], 0, ((i % 6) * 32), (((i / 6) * 32) + 128), 32, 32);
    for (int i = 0; i < 12; i++)
        noodleOffIcon[i] = init_sprite(&oamSub, &nextIdSub, gfxSub[28 + i], 0, ((i % 6) * 32), (((i / 6) * 32) + 128), 32, 32);

    // 4.3. Update hardware OAM
    oamUpdate(&oamMain);
    oamUpdate(&oamSub);

    // 5. Initialize backgrounds
    // extended palettes only work on 8bpp tiled bg's with 16-bit map entries
    // this means you can only use BgType_Text8bpp or BgType_ExRotation
    // http://mtheall.com/vram.html#T0=1&NT0=192&MB0=6&TB0=0&S0=0&T1=1&NT1=576&MB1=7&TB1=1&S1=0
    //
    // Background map allocation (mapBase) is chunked into 2KB blocks,
    // while tile/gfx allocation (tileBase) is chunked into 16KB blocks.
    // This means we choose tileBase indices first:
    // tileBase 0 = address 0KB
    // tileBase 1 = address 16KB
    // tileBase 2 = address 32KB
    // ..and do a little calculation to place each bg's mapBase (usually ~2KB) at
    // the last mapBase index inside each backgrounds tileBase (for ease of setup):
    // mapBase = (((tileBase + 1) * 16KB) - 2KB) / 2KB = (tileBase * 8) / 7
    // ..This is possible since none of these background tiles exceed more than 14KB,
    // so we can squash their maps into the space at the end of their tile allocation block.
    logoBg = bgInit(1, BgType_Text8bpp, BgSize_T_256x256, ((0 * 8) + 7), 0);
    controlHintsBg = bgInit(2, BgType_Text8bpp, BgSize_T_256x256, ((1 * 8) + 7), 1);
    backgroundBgMain = bgInit(3, BgType_Text8bpp, BgSize_T_256x256, ((2 * 8) + 7), 2);
    difficultyBg = bgInitSub(1, BgType_Text8bpp, BgSize_T_256x256, ((0 * 8) + 7), 0);
    boardBg = bgInitSub(2, BgType_Text8bpp, BgSize_T_256x256, ((1 * 8) + 7), 1);
    backgroundBgSub = bgInitSub(3, BgType_Text8bpp, BgSize_T_256x256, ((2 * 8) + 7), 2);

    // copy graphics to vram
    dmaCopy(logoTiles, bgGetGfxPtr(logoBg), logoTilesLen);
    dmaCopy(controlHintsTiles, bgGetGfxPtr(controlHintsBg), controlHintsTilesLen);
    dmaCopy(backgroundTiles, bgGetGfxPtr(backgroundBgMain), backgroundTilesLen);
    dmaCopy(difficultyTiles, bgGetGfxPtr(difficultyBg), difficultyTilesLen);
    dmaCopy(boardTiles, bgGetGfxPtr(boardBg), boardTilesLen);
    dmaCopy(backgroundTiles, bgGetGfxPtr(backgroundBgSub), backgroundTilesLen);

    // copy maps to vram
    dmaCopy(logoMap, bgGetMapPtr(logoBg), logoMapLen);
    dmaCopy(controlHintsMap, bgGetMapPtr(controlHintsBg), controlHintsMapLen);
    dmaCopy(backgroundMap, bgGetMapPtr(backgroundBgMain), backgroundMapLen);
    dmaCopy(difficultyMap, bgGetMapPtr(difficultyBg), difficultyMapLen);
    dmaCopy(boardMap, bgGetMapPtr(boardBg), boardMapLen);
    dmaCopy(backgroundMap, bgGetMapPtr(backgroundBgSub), backgroundMapLen);

    // you can only access extended palettes in LCD mode
    vramSetBankE(VRAM_E_LCD); // for main engine
    vramSetBankH(VRAM_H_LCD); // for sub engine

    // copy palettes to extended palette area
    // there are 16 256-color palettes per bg
    // use '-mp #' to make grit use # for the slot number
    dmaCopy(logoPal, &VRAM_E_EXT_PALETTE[1][4], logoPalLen);                 // bg 1, slot 4
    dmaCopy(controlHintsPal, &VRAM_E_EXT_PALETTE[2][2], controlHintsPalLen); // bg 2, slot 2
    dmaCopy(backgroundPal, &VRAM_E_EXT_PALETTE[3][0], backgroundPalLen);     // bg 3, slot 0
    dmaCopy(difficultyPal, &VRAM_H_EXT_PALETTE[1][3], difficultyPalLen);     // bg 1, slot 3
    dmaCopy(boardPal, &VRAM_H_EXT_PALETTE[2][1], boardPalLen);               // bg 2, slot 1
    dmaCopy(backgroundPal, &VRAM_H_EXT_PALETTE[3][0], backgroundPalLen);     // bg 3, slot 0

    // map vram banks to extended palettes
    // http://mtheall.com/banks.html#A=MBG0&C=MBG2&E=BGEPAL&H=SBGEPAL
    vramSetBankE(VRAM_E_BG_EXT_PALETTE);     // for main engine
    vramSetBankH(VRAM_H_SUB_BG_EXT_PALETTE); // for sub engine
}