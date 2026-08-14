#pragma once

#include <stdio.h>
#include "polyomino.h"

#define NOODLE_A (Polyomino){ .id = 'A', .size = 4, .cell = {{0,0},{1,0},{2,0},{0,1}} }
#define NOODLE_B (Polyomino){ .id = 'B', .size = 5, .cell = {{0,0},{1,0},{2,0},{0,1},{1,1}} }
#define NOODLE_C (Polyomino){ .id = 'C', .size = 5, .cell = {{0,0},{1,0},{2,0},{3,0},{0,1}} }
#define NOODLE_D (Polyomino){ .id = 'D', .size = 5, .cell = {{0,0},{1,0},{2,0},{3,0},{1,1}} }
#define NOODLE_E (Polyomino){ .id = 'E', .size = 5, .cell = {{0,0},{1,0},{2,0},{2,1},{3,1}} }
#define NOODLE_F (Polyomino){ .id = 'F', .size = 3, .cell = {{0,0},{1,0},{0,1}} }
#define NOODLE_G (Polyomino){ .id = 'G', .size = 5, .cell = {{0,0},{1,0},{2,0},{0,1},{0,2}} }
#define NOODLE_H (Polyomino){ .id = 'H', .size = 5, .cell = {{0,0},{1,0},{1,1},{2,1},{2,2}} }
#define NOODLE_I (Polyomino){ .id = 'I', .size = 5, .cell = {{0,0},{1,0},{2,0},{0,1},{2,1}} }
#define NOODLE_J (Polyomino){ .id = 'J', .size = 4, .cell = {{0,0},{1,0},{2,0},{3,0}} }
#define NOODLE_K (Polyomino){ .id = 'K', .size = 4, .cell = {{0,0},{1,0},{0,1},{1,1}} }
#define NOODLE_L (Polyomino){ .id = 'L', .size = 5, .cell = {{1,0},{0,1},{1,1},{2,1},{1,2}} }

static const Polyomino NOODLES[] = {
    NOODLE_A, NOODLE_B, NOODLE_C, NOODLE_D, 
    NOODLE_E, NOODLE_F, NOODLE_G, NOODLE_H, 
    NOODLE_I, NOODLE_J, NOODLE_K, NOODLE_L};

#define NOODLE_COUNT sizeof(NOODLES) / sizeof(Polyomino)
#define BOARD_WIDTH 11
#define BOARD_HEIGHT 5
#define MOVES_SAMPLE 900 // sample size of move-space

size_t sample_move_space(Polyomino sample[], int k);
void print_solution(const Polyomino *solution, size_t size);
size_t random_solution(Polyomino *solution);