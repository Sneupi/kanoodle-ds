/*---------------------------------------------------------------------------------

	Basic template code for starting a DS app

---------------------------------------------------------------------------------*/
#include <nds.h>
#include <stdio.h>
#include "kanoodle.h"

//---------------------------------------------------------------------------------
int main(int argc, char* argv[]) {
//---------------------------------------------------------------------------------

	consoleDemoInit();
	int n = 0;

	while(pmMainLoop()) {
		swiWaitForVBlank();
		scanKeys();
		int pressed = keysDown();
		if(pressed & KEY_START) break;

		if (pressed & KEY_TOUCH) {
			iprintf("\x1b[0;0H"); // goto 0,0
			Polyomino sol[12];
			random_solution(sol);
			print_solution(sol);
			printf("SOL#: %d\n", n);
			n++;
		}
	}

	return 0;

}
