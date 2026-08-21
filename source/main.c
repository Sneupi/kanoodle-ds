
#include <nds.h>
#include "graphics.h"

int main(void)
{
    init_graphics();

    menu_screen();

    touchPosition touch;
    bool state_screen = 0;

    // 5. Main Game Loop
    while (1)
    {
        swiWaitForVBlank();

        scanKeys();
        touchRead(&touch);        // read the touchscreen coordinates
        int pressed = keysDown(); // buttons pressed this loop
        int held = keysHeld();    // buttons currently held

        if (pressed & KEY_TOUCH)
        {
            state_screen = !state_screen;
            if (state_screen)
                menu_screen();
            else
                game_screen();
        }

        bgUpdate();
        oamUpdate(&oamMain);
        oamUpdate(&oamSub);
    }

    return 0;
}