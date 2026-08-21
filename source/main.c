
#include <nds.h>
#include "graphics.h"

int main(void)
{
    init_graphics();

    // 5. Main Game Loop
    while (1)
    {
        swiWaitForVBlank();
        bgUpdate();

        // Ensure OAM registers are updated every frame
        oamUpdate(&oamMain);
        oamUpdate(&oamSub);
    
    }

    return 0;
}