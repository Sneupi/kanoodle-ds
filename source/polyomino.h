
#pragma once

#include <stdint.h>

typedef struct
{
    // (Useful metadata)
    uint8_t id;

    // Num cells
    uint8_t size;

    /*
     * Cell coordinates (x,y)
     *
     * NOTE: Array size is extendible
     *       without breaking lib
     */
    struct
    {
        int16_t x;
        int16_t y;
    } cell[5];

} Polyomino;

/**
 * Gets min and max (x,y) bounds of a polyomino
 *
 * Returns: 0 (success), -1 (empty shape)
 */
int bounds(Polyomino p, int *x0, int *y0, int *x1, int *y1);

// Finds width of a polyomino
int width(Polyomino p);

// Finds height of a polyomino
int height(Polyomino p);

// Shifts polyomino by delta
Polyomino shifted(Polyomino poly, int dx, int dy);

// Returns a polyomino, shifting its min-bounds to (0,0)
Polyomino zero_bounded(Polyomino p);

// Flips a polyomino in-place vertically e.g. ABC -> CBA
Polyomino flipped(Polyomino p);

// Rotate a polyomino in-place clockwise 90-degree assuming a (+,+) cartesian Quadrant IV
Polyomino rotated(Polyomino p);

// Returns 1 if size and coordinate sets match, else 0
int is_equal(Polyomino p1, Polyomino p2);

// Returns 1 if polyomino contains (x,y) coord, else 0
int contains(Polyomino p, int x, int y);

/**
 * Loads a list of all zero-bounded orientations
 * of a polyomino into the provided array pointer.
 *
 * Returns: size of arr (success), -1 (arr prev init), -2 (bad alloc).
 *
 * NOTE: Array is allocated with calloc() and must be free()'d.
 */
int get_orientations(Polyomino poly, Polyomino **arr);

// prints out polyomino as a string with specified cartesian bounds
void print_poly_bounded(Polyomino p, int x0, int y0, int x1, int y1);

// prints out polyomino as a string
void print_poly(Polyomino p);
