#include <nds.h>
#include "graphics.h"
#include "kanoodle.h"

#include "polyBead.h"
#include "polyIcon.h"

// int init_bg(BackgroundConfig *bg) {
//     int id;

//     // init bg in vram
//     if (bg->isMain)
//         id = bgInit(bg->layer, bg->type, bg->size, bg->mapBase, bg->tileBase);
//     else
//         id = bgInitSub(bg->layer, bg->type, bg->size, bg->mapBase, bg->tileBase);

//     // load gfx
//     dmaCopy(bg->tiles, bgGetGfxPtr(id), bg->tilesLen);
//     dmaCopy(bg->map, bgGetMapPtr(id), bg->mapLen);

//     // copy palettes to extended palette area
//     // there are 16 256-color palettes per bg
//     // use '-mp #' to make grit use # for the slot number
//     if (bg->isMain) {
//         // Use Bank E as Ext Palette Main
//         vramSetBankE(VRAM_E_LCD); // write-mode
//         dmaCopy(bg->pal, &VRAM_E_EXT_PALETTE[bg->layer][bg->palSlot], bg->palLen);
//         vramSetBankE(VRAM_E_BG_EXT_PALETTE);
//     }
//     else {
//         // Use Bank H as Ext Palette Sub
//         vramSetBankH(VRAM_H_LCD); // write-mode
//         dmaCopy(bg->pal, &VRAM_H_EXT_PALETTE[bg->layer][bg->palSlot], bg->palLen);
//         vramSetBankH(VRAM_H_SUB_BG_EXT_PALETTE);
//     }

//     return id;
// }

void init_config()
{
    // Set display modes
    videoSetMode(MODE_0_2D);
    videoSetModeSub(MODE_0_2D);

    // Allocate VRAM Banks
    vramSetBankA(VRAM_A_MAIN_BG);
    vramSetBankB(VRAM_B_MAIN_SPRITE);
    vramSetBankC(VRAM_C_SUB_BG);
    vramSetBankD(VRAM_D_SUB_SPRITE);

    // Enable extended palettes
    bgExtPaletteEnable();
    bgExtPaletteEnableSub();

    // Init OAM sprite engine
    oamInit(&oamMain, SpriteMapping_1D_32, false);
    oamInit(&oamSub, SpriteMapping_1D_32, false);
}

int main(void)
{

    init_config();

    SpriteTemplate polyBeadTemplate = {
        .size = SpriteSize_32x32,
        .colorFormat = SpriteColorFormat_16Color,
        .tiles = polyBeadTiles,
        .tilesLen = polyBeadTilesLen,
        .pal = polyBeadPal,
        .palLen = polyBeadPalLen,
    };
    SpriteGfx polyBeadGfx = init_sprite_gfx(&oamMain, polyBeadTemplate);
    PolySprite noodleAPolySprite = init_poly_sprite(NOODLE_A, polyBeadGfx);

    while (1)
    {
        swiWaitForVBlank();

        // Update hardware OAM registers during VBlank
        oamUpdate(&oamMain);
        oamUpdate(&oamSub);
    }

    return 0;
}