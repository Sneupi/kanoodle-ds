#pragma once

#include <stdio.h>
#include "polyomino.h"

#define BOARD_WIDTH 11
#define BOARD_HEIGHT 5

// get noodle constant by it's char id
#define NOODLE(id) NOODLES[id-'A'] 

// example orientations of each kanoodle piece
extern const Polyomino NOODLES[];
extern const size_t NOODLES_SIZE;

// get random sample of k moves from kanoodle move-space
size_t sample_move_space(Polyomino sample[], int k);

// print array of pieces as text 
void print_solution(const Polyomino *solution, size_t size);

// get random solution sampled from k moves in kanoodle move-space
size_t random_solution(Polyomino *solution, int k);