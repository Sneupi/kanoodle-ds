
#include <nds.h>
#include "graphics.h"

int main(void)
{
    init_graphics();

    game_screen();

    // 5. Main Game Loop
    while (1)
    {
        swiWaitForVBlank();

        handle_input();

        bgUpdate();
        oamUpdate(&oamMain);
        oamUpdate(&oamSub);
    }

    return 0;
}