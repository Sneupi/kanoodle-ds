
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

// oam representation
int spriteCountMain = 0;
int spriteCountSub = 0;
Sprite spritesMain[128];
Sprite spritesSub[128];

// initialized assets
Sprite *sub_sp_flip;
Sprite *sub_sp_rotate;
Sprite *sub_sp_panel;
PolySprite sub_pl_highlight;
PolySprite main_pl_noodle[12];
PolySprite sub_pl_noodles[12];
Sprite *sub_sp_panel_off[12];
Sprite *sub_sp_panel_on[12];
int main_bg_logo;
int main_bg_control_hints;
int main_bg_background;
int sub_bg_difficulty;
int sub_bg_board;
int sub_bg_background;

// graphics state
bool state_panel_hide = 0;            // 0 (show), 1 (hide)
bool state_panel[12] = {0};           // 0 (noodle off), 1(noodle on)
PolySprite *state_highlighted = NULL; // currently worked piece

/**
 * Helpers
 */

Sprite *init_sprite(OamState *oam, u16 *gfx, int priority, int x, int y, int w, int h, void (*callback)(Sprite *), int polyId)
{
    // Assign unique non-overlapping IDs via global counter
    int id = (oam == &oamMain) ? spriteCountMain++ : spriteCountSub++;
    bool hide = false;

    // NOTE: sprite sizes standardized, thus hardcode SpriteSize & SpriteColorFormat
    oamSet(
        oam,                        // Main or Sub OAM
        id,                         // OAM entry ID (0 to 127)
        x, y,                       // Screen coordinates
        priority,                   // Priority (0-3)
        0,                          // Palette index (0 for 256-color)
        SpriteSize_32x32,           // Sprite dimensions
        SpriteColorFormat_256Color, // Color depth
        gfx,                        // Graphics pointer in VRAM
        -1,                         // Affine index (-1 = no rotation/scaling)
        false,                      // Double size flag
        hide,                       // Hide sprite flag
        false, false,               // VFlip, HFlip
        false                       // Mosaic
    );

    Sprite sp = (Sprite){
        .x = x,
        .y = y,
        .w = w,
        .h = h,
        .oam = oam,
        .oamId = id,
        .polyId = polyId,
        .callback = callback,
        .hidden = hide,
    };

    if (oam == &oamMain)
    {
        spritesMain[id] = sp;
        return &spritesMain[id];
    }
    else
    {
        spritesSub[id] = sp;
        return &spritesSub[id];
    }
}

PolySprite init_poly_sprite(Polyomino p, OamState *oam, u16 *gfx, int priority, int x, int y, int cell_w_px, int cell_h_px)
{
    PolySprite ps = {
        .x = x,
        .y = y,
        .poly = zero_bounded(p),
        .highlight = 0,
    };

    for (int i = 0; i < ps.poly.size; i++)
    {
        int xOffset = x + (cell_w_px * ps.poly.cell[i].x);
        int yOffset = y + (cell_h_px * ps.poly.cell[i].y);
        ps.sprites[i] = init_sprite(oam, gfx, priority, xOffset, yOffset, cell_w_px, cell_h_px, cb_highlight_draggable, (int)ps.poly.id);
    }
    return ps;
}

/**
 * Game Graphics
 */
void init_graphics()
{

    // 1. Initialize Video & VRAM
    videoSetMode(MODE_0_2D);
    videoSetModeSub(MODE_0_2D);

    vramSetBankA(VRAM_A_MAIN_BG);
    vramSetBankB(VRAM_B_MAIN_SPRITE);
    vramSetBankC(VRAM_C_SUB_BG);
    vramSetBankD(VRAM_D_SUB_SPRITE);

    // enable ext bg palettes for flexibility
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
    sub_sp_flip = init_sprite(&oamSub, gfxSub[0], 0, 224, 160, 32, 32, cb_flip_highlighted, -1);
    sub_sp_rotate = init_sprite(&oamSub, gfxSub[1], 0, 224, 128, 32, 32, cb_rotate_highlighted, -1);
    sub_sp_panel = init_sprite(&oamSub, gfxSub[2], 0, 192, 160, 32, 32, cb_toggle_panel, -1);
    for (int i = 0; i < 12; i++)
        sub_sp_panel_off[i] = init_sprite(&oamSub, gfxSub[16 + i], 0, ((i % 6) * 32), (((i / 6) * 32) + 128), 32, 32, cb_toggle_noodle, (int)NOODLES[i].id);
    for (int i = 0; i < 12; i++)
        sub_sp_panel_on[i] = init_sprite(&oamSub, gfxSub[28 + i], 0, ((i % 6) * 32), (((i / 6) * 32) + 128), 32, 32, cb_toggle_noodle, (int)NOODLES[i].id);
    for (int i = 0; i < 12; i++)
        main_pl_noodle[i] = init_poly_sprite(NOODLES[i], &oamMain, gfxMain[4 + i], 0, 100, 100, 20, 20);
    for (int i = 0; i < 12; i++)
        sub_pl_noodles[i] = init_poly_sprite(NOODLES[i], &oamSub, gfxSub[4 + i], 0, 100, 100, 20, 20);
    sub_pl_highlight = init_poly_sprite((Polyomino){.id = '?', .size = MAX_POLY_CELLS, .cell = {0}}, &oamSub, gfxSub[3], 0, 0, 0, 20, 20);

    // 4.3. Generate initial placement layout once
    Polyomino sol[12];
    random_solution(sol, 900);
    for (int i = 0; i < 12; i++)
    {
        for (int j = 0; j < 12; j++)
        {
            if (main_pl_noodle[i].poly.id == sol[j].id)
            {
                main_pl_noodle[i].poly = sol[j];
                place_poly_sprite(&main_pl_noodle[i], 18, 16);
            }
        }
    }

    // 4.4. Update hardware OAM
    oamUpdate(&oamMain);
    oamUpdate(&oamSub);

    // 5. Initialize backgrounds
    // extended palettes only work on 8bpp tiled bg's with 16-bit map entries
    // this means you can only use BgType_Text8bpp or BgType_ExRotation
    // http://mtheall.com/vram.html#T0=1&NT0=192&MB0=6&TB0=0&S0=0&T1=1&NT1=576&MB1=7&TB1=1&S1=0
    //
    // Background map allocation (mapBase) is chunked into 2KB blocks/index
    // while tile/gfx allocation (tileBase) is chunked into 16KB blocks/index
    // This means we choose the tileBase indices first for each bg:
    // tileBase 0 = address 0KB
    // tileBase 1 = address 16KB
    // tileBase 2 = address 32KB
    // ..and do a little calculation to place each bg's mapBase (usually ~2KB) at
    // the highest mapBase index inside each bg's tileBase vram allocation (for ease of setup & understanding):
    // | TILEBASE BEGIN | <=14KB of Tiles | (empty space) | <= 2KB Map | TILEBASE END |
    // mapBase = (((tileBase + 1) * 16KB) - 2KB) / 2KB = (tileBase * 8) / 7
    // ..This is possible since none of the used background tiles exceed more than 14KB,
    // so we can squeeze their map allocation into the space at the end of their tiles allocated chunk space
    // for all backgrounds in this game.
    main_bg_logo = bgInit(1, BgType_Text8bpp, BgSize_T_256x256, ((0 * 8) + 7), 0);
    main_bg_control_hints = bgInit(2, BgType_Text8bpp, BgSize_T_256x256, ((1 * 8) + 7), 1);
    main_bg_background = bgInit(3, BgType_Text8bpp, BgSize_T_256x256, ((2 * 8) + 7), 2);
    sub_bg_difficulty = bgInitSub(1, BgType_Text8bpp, BgSize_T_256x256, ((0 * 8) + 7), 0);
    sub_bg_board = bgInitSub(2, BgType_Text8bpp, BgSize_T_256x256, ((1 * 8) + 7), 1);
    sub_bg_background = bgInitSub(3, BgType_Text8bpp, BgSize_T_256x256, ((2 * 8) + 7), 2);

    // copy graphics to vram
    dmaCopy(logoTiles, bgGetGfxPtr(main_bg_logo), logoTilesLen);
    dmaCopy(controlHintsTiles, bgGetGfxPtr(main_bg_control_hints), controlHintsTilesLen);
    dmaCopy(backgroundTiles, bgGetGfxPtr(main_bg_background), backgroundTilesLen);
    dmaCopy(difficultyTiles, bgGetGfxPtr(sub_bg_difficulty), difficultyTilesLen);
    dmaCopy(boardTiles, bgGetGfxPtr(sub_bg_board), boardTilesLen);
    dmaCopy(backgroundTiles, bgGetGfxPtr(sub_bg_background), backgroundTilesLen);

    // copy maps to vram
    dmaCopy(logoMap, bgGetMapPtr(main_bg_logo), logoMapLen);
    dmaCopy(controlHintsMap, bgGetMapPtr(main_bg_control_hints), controlHintsMapLen);
    dmaCopy(backgroundMap, bgGetMapPtr(main_bg_background), backgroundMapLen);
    dmaCopy(difficultyMap, bgGetMapPtr(sub_bg_difficulty), difficultyMapLen);
    dmaCopy(boardMap, bgGetMapPtr(sub_bg_board), boardMapLen);
    dmaCopy(backgroundMap, bgGetMapPtr(sub_bg_background), backgroundMapLen);

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

void menu_screen()
{
    // HIDE
    hide_sprite(sub_sp_flip, true);
    hide_sprite(sub_sp_rotate, true);
    hide_sprite(sub_sp_panel, true);
    hide_poly_sprite(&sub_pl_highlight, true);
    for (int i = 0; i < 12; i++)
    {
        hide_poly_sprite(&main_pl_noodle[i], true);
        hide_poly_sprite(&sub_pl_noodles[i], true);
    }
    state_panel_hide = 0;
    cb_toggle_panel(NULL); // hides on toggle
    hide_bg(main_bg_control_hints, true);
    hide_bg(sub_bg_board, true);

    // SHOW
    hide_bg(main_bg_logo, false);
    hide_bg(main_bg_background, false);
    hide_bg(sub_bg_difficulty, false);
    hide_bg(sub_bg_background, false);
}

void game_screen()
{
    // HIDE
    hide_poly_sprite(&sub_pl_highlight, true);
    for (int i = 0; i < 12; i++)
    {
        hide_poly_sprite(&sub_pl_noodles[i], true);
    }
    state_panel_hide = 0;
    cb_toggle_panel(NULL); // hides on toggle
    hide_bg(main_bg_logo, true);
    hide_bg(sub_bg_difficulty, true);

    // SHOW
    hide_sprite(sub_sp_flip, false);
    hide_sprite(sub_sp_rotate, false);
    hide_sprite(sub_sp_panel, false);
    for (int i = 0; i < 12; i++)
    {
        hide_poly_sprite(&main_pl_noodle[i], false);
    }
    hide_bg(main_bg_control_hints, false);
    hide_bg(main_bg_background, false);
    hide_bg(sub_bg_board, false);
    hide_bg(sub_bg_background, false);
}

/**
 * Callbacks
 */

void cb_toggle_panel(Sprite * /*unused*/)
{
    state_panel_hide = !state_panel_hide;
    for (int i = 0; i < 12; i++)
    {
        if (state_panel_hide)
        {
            hide_sprite(sub_sp_panel_on[i], true);
            hide_sprite(sub_sp_panel_off[i], true);
        }
        else
        {
            hide_sprite((state_panel[i]) ? sub_sp_panel_on[i] : sub_sp_panel_off[i], false);
        }
    }
}

void cb_toggle_noodle(Sprite *s)
{
    int i = s->polyId - 'A';
    hide_sprite((state_panel[i]) ? sub_sp_panel_on[i] : sub_sp_panel_off[i], true);
    state_panel[i] = !state_panel[i];
    hide_sprite((state_panel[i]) ? sub_sp_panel_on[i] : sub_sp_panel_off[i], false);
    hide_poly_sprite(&sub_pl_noodles[i], !state_panel[i]);

    // if noodle is highlighted and being hidden, unhighlight
    if ((state_highlighted == &sub_pl_noodles[i]) && !state_panel[i])
    {
        hide_poly_sprite(&sub_pl_highlight, true);
        state_highlighted = NULL;
    }
}

void cb_highlight_draggable(Sprite *s)
{
    // unselect prev piece
    if (state_highlighted)
        state_highlighted->highlight = false;

    // point to new piece
    state_highlighted = &sub_pl_noodles[s->polyId - 'A'];
    state_highlighted->highlight = true;

    // update polysprite to highlight new
    for (int i = 0; i < MAX_POLY_CELLS; i++)
        hide_sprite(sub_pl_highlight.sprites[i], (i >= state_highlighted->poly.size));
    sub_pl_highlight.poly = state_highlighted->poly;
    place_poly_sprite(&sub_pl_highlight, state_highlighted->x - 1, state_highlighted->y - 1);
}
void cb_flip_highlighted(Sprite *s) {}   // FIXME impl
void cb_rotate_highlighted(Sprite *s) {} // FIXME impl

void handle_input()
{
    touchPosition touch;
    scanKeys();
    touchRead(&touch);        // read the touchscreen coordinates
    int pressed = keysDown(); // buttons pressed this loop
    int held = keysHeld();    // buttons currently held

    // touchscreen press
    if (pressed & KEY_TOUCH)
    {
        int x = touch.px;
        int y = touch.py;

        // iter sub sprites (touchscreen sprites)
        for (int i = 0; i < spriteCountSub; i++)
        {
            Sprite *sp = &spritesSub[i];
            int x0 = sp->x;
            int y0 = sp->y;
            int x1 = sp->x + sp->w;
            int y1 = sp->y + sp->h;

            if (x >= x0 &&
                x <= x1 &&
                y >= y0 &&
                y <= y1 &&
                !sp->hidden)
            {
                if (sp->callback)
                    sp->callback(sp);

                break;
            }
        }
    }
}

/**
 * Sprites
 */

void hide_sprite(Sprite *s, bool hide)
{
    s->hidden = hide;
    oamSetHidden(s->oam, s->oamId, hide);
}

void place_sprite(Sprite *s, int x, int y)
{
    oamSetXY(s->oam, s->oamId, x, y);
}

void shift_sprite(Sprite *s, int dx, int dy)
{
    place_sprite(s, s->x + dx, s->y + dy);
}

/**
 * PolySprites
 */

void hide_poly_sprite(PolySprite *s, bool hide)
{
    for (int i = 0; i < s->poly.size; i++)
        hide_sprite(s->sprites[i], hide);
}

void place_poly_sprite(PolySprite *s, int x, int y)
{
    s->x = x;
    s->y = y;
    for (int i = 0; i < s->poly.size; i++)
    {
        Sprite *spr = s->sprites[i];
        int xOffset = x + (spr->w * s->poly.cell[i].x);
        int yOffset = y + (spr->h * s->poly.cell[i].y);
        place_sprite(spr, xOffset, yOffset);
    }
}

void shift_poly_sprite(PolySprite *s, int dx, int dy)
{
    place_poly_sprite(s, s->x + dx, s->y + dy);
}

void rotate_poly_sprite(PolySprite *s)
{
    s->poly = zero_bounded(rotated(s->poly));
    place_poly_sprite(s, s->x, s->y);
}

void flip_poly_sprite(PolySprite *s)
{
    s->poly = zero_bounded(flipped(s->poly));
    place_poly_sprite(s, s->x, s->y);
}

/**
 * Backgrounds
 */

void hide_bg(int id, bool hide)
{
    if (hide)
        bgHide(id);
    else
        bgShow(id);
}