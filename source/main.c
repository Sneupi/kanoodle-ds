
#include <nds.h>
#include "sprites.h"

// Arrays to store VRAM pointers for each sprite frame
u16* mainGfxPointers[40];
u16* subGfxPointers[40];

void initAndDisplaySprites(void) {
    int spriteSizeInBytes = 32 * 32; // 1024 bytes for 8-bit 32x32
    
    // 1. Allocate VRAM memory for all 40 frames
    for (int i = 0; i < 40; i++) {
        mainGfxPointers[i] = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_256Color);
        subGfxPointers[i]  = oamAllocateGfx(&oamSub,  SpriteSize_32x32, SpriteColorFormat_256Color);

        // Copy the specific frame slice from the sprite sheet array
        u8* frameOffset = (u8*)spritesTiles + (i * spriteSizeInBytes);
        dmaCopy(frameOffset, mainGfxPointers[i], spriteSizeInBytes);
        dmaCopy(frameOffset, subGfxPointers[i],  spriteSizeInBytes);
    }

    // 2. Set OAM entry attributes to place them on screen in a 5x4 grid
    for (int i = 0; i < 40; i++) {
        // Render frames 0-19 on Main Screen, frames 20-39 on Sub Screen
        OamState* targetOam = (i < 20) ? &oamMain : &oamSub;
        u16** gfxPointers   = (i < 20) ? mainGfxPointers : subGfxPointers;
        int localIndex      = i % 20;

        // Grid calculation: 5 columns x 4 rows per screen
        int x = (localIndex % 5) * 48 + 8;  // Spaced across screen width (256px)
        int y = (localIndex / 5) * 40 + 8;  // Spaced across screen height (192px)

        oamSet(
            targetOam,                   // Main or Sub OAM
            localIndex,                  // OAM entry ID (0 to 19)
            x, y,                        // Screen coordinates
            3,                           // Priority (0-3)
            0,                           // Palette index (0 for 256-color)
            SpriteSize_32x32,            // Sprite dimensions
            SpriteColorFormat_256Color,  // Color depth
            gfxPointers[i],              // Graphics pointer in VRAM
            -1,                          // Affine index (-1 = no rotation/scaling)
            false,                       // Double size flag
            false,                       // Hide sprite flag
            false, false,                // VFlip, HFlip
            false                        // Mosaic
        );
    }

    // 3. Update hardware OAM
    oamUpdate(&oamMain);
    oamUpdate(&oamSub);
}

int main(void) {
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
    oamInit(&oamSub,  SpriteMapping_1D_128, false);

    // 3. Copy Palettes
    dmaCopy(spritesPal, SPRITE_PALETTE, spritesPalLen);
    dmaCopy(spritesPal, SPRITE_PALETTE_SUB, spritesPalLen);

    // 4. Allocate & Setup Sprites
    initAndDisplaySprites();

    // 5. Main Game Loop
    while (1) {
        swiWaitForVBlank();
        
        // Ensure OAM registers are updated every frame
        oamUpdate(&oamMain);
        oamUpdate(&oamSub);
    }

    return 0;
}