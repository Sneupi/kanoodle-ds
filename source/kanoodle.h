#pragma once

#include "polyomino.h"

/**
 * Let's define some Kanoodle-style polyominoes "noodles"
 *
 * ┏━━━━━━━━━┳━━━━━━━━━━━━━━━━━━━┳━━━━━━━━━┳━━━━━━━━━━━━━━┓
 * ┃      F  ┃  C                ┃         ┃              ┃
 * ┣━━━━┓    ┃    ┏━━━━━━━━━┳━━━━┻━━━━┓    ┗━━━━┓     B   ┃
 * ┃    ┃    ┃    ┃         ┃         ┃  H      ┃         ┃
 * ┃    ┣━━━━┻━━━━┛    ┏━━━━┫    K    ┣━━━━┓    ┣━━━━┳━━━━┫
 * ┃    ┃           E  ┃    ┃         ┃    ┃    ┃    ┃    ┃
 * ┃    ┗━━━━━━━━━┳━━━━┛    ┗━━━━┳━━━━┫    ┗━━━━┛    ┃    ┃
 * ┃  G           ┃       L      ┃    ┃       I      ┃    ┃
 * ┣━━━━━━━━━━━━━━┻━━━━┓    ┏━━━━┛    ┗━━━━━━━━━┳━━━━┛    ┃
 * ┃         J         ┃    ┃       D           ┃      A  ┃
 * ┗━━━━━━━━━━━━━━━━━━━┻━━━━┻━━━━━━━━━━━━━━━━━━━┻━━━━━━━━━┛
 */

#define NOODLE_A (Polyomino){'A', 4, {{0,0},{1,0},{2,0},{0,1}}}
#define NOODLE_B (Polyomino){'B', 5, {{0,0},{1,0},{2,0},{0,1},{1,1}}}
#define NOODLE_C (Polyomino){'C', 5, {{0,0},{1,0},{2,0},{3,0},{0,1}}}
#define NOODLE_D (Polyomino){'D', 5, {{0,0},{1,0},{2,0},{3,0},{1,1}}}
#define NOODLE_E (Polyomino){'E', 5, {{0,0},{1,0},{2,0},{2,1},{3,1}}}
#define NOODLE_F (Polyomino){'F', 3, {{0,0},{1,0},{0,1}}}
#define NOODLE_G (Polyomino){'G', 5, {{0,0},{1,0},{2,0},{0,1},{0,2}}}
#define NOODLE_H (Polyomino){'H', 5, {{0,0},{1,0},{1,1},{2,1},{2,2}}}
#define NOODLE_I (Polyomino){'I', 5, {{0,0},{1,0},{2,0},{0,1},{2,1}}}
#define NOODLE_J (Polyomino){'J', 4, {{0,0},{1,0},{2,0},{3,0}}}
#define NOODLE_K (Polyomino){'K', 4, {{0,0},{1,0},{0,1},{1,1}}}
#define NOODLE_L (Polyomino){'L', 5, {{1,0},{0,1},{1,1},{2,1},{1,2}}}

typedef struct {

    // cell values 0-11 (A-L)
    char grid[11][5]; 

} KanoodleSol;

/**
 * Generate a random Kanoodle game solution
 * for an 11x5 board and 12 standard pieces
 */
KanoodleSol random_solution();

// Print solution as ASCII string
void print_solution(KanoodleSol sol);
