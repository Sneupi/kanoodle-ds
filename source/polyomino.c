#include "polyomino.h"

#include <stdlib.h>
#include <stdio.h>

int bounds(Polyomino p, int *x0, int *y0, int *x1, int *y1)
{
    if (p.size <= 0)
        return -1;

    *x0 = p.cell[0].x;
    *y0 = p.cell[0].y;
    *x1 = p.cell[0].x;
    *y1 = p.cell[0].y;

    for (int i = 1; i < p.size; i++)
    {
        if (p.cell[i].x < *x0)
            *x0 = p.cell[i].x;
        if (p.cell[i].y < *y0)
            *y0 = p.cell[i].y;
        if (p.cell[i].x > *x1)
            *x1 = p.cell[i].x;
        if (p.cell[i].y > *y1)
            *y1 = p.cell[i].y;
    }
    return 0;
}

int width(Polyomino p)
{
    int x0, y0, x1, y1;
    if (-1 == bounds(p, &x0, &y0, &x1, &y1))
        return 0;
    return x1 - x0 + 1;
}

int height(Polyomino p)
{
    int x0, y0, x1, y1;
    if (-1 == bounds(p, &x0, &y0, &x1, &y1))
        return 0;
    return y1 - y0 + 1;
}

Polyomino shifted(Polyomino poly, int dx, int dy)
{
    for (int i = 0; i < poly.size; i++)
    {
        poly.cell[i].x += dx;
        poly.cell[i].y += dy;
    }
    return poly;
}

Polyomino zero_bounded(Polyomino p)
{
    if (p.size <= 0)
        return p;

    // find min bounds
    int x0, y0, x1, y1;
    bounds(p, &x0, &y0, &x1, &y1);

    // return shifted by min bounds
    return shifted(p, -x0, -y0);
}

Polyomino flipped(Polyomino p)
{
    // find bounds for flip in-place (about min bounds)
    int x0, y0, x1, y1;
    if (-1 == bounds(p, &x0, &y0, &x1, &y1))
        return p;

    // vertically: (x,y) -> (-x,y)
    for (int i = 0; i < p.size; i++)
        p.cell[i].x = -p.cell[i].x;

    // return in-place flip
    return shifted(p, x0 + x1, 0);
}

Polyomino rotated(Polyomino p)
{
    // find bounds for rotating in-place (about min bounds)
    int x0, y0, x1, y1;
    bounds(p, &x0, &y0, &x1, &y1);

    // for (+,+) coordinates in Quadrant IV
    // clockwise 90deg: (x,y) -> (-y,x)
    for (int i = 0; i < p.size; i++)
    {
        int x = p.cell[i].x;
        p.cell[i].x = -p.cell[i].y;
        p.cell[i].y = x;
    }

    // shift required for in-place rotation
    int dx = x0 - (-y1);
    int dy = y0 - x0;

    // return in-place rotation
    return shifted(p, dx, dy);
}

int is_equal(Polyomino p1, Polyomino p2)
{
    if (p1.size != p2.size)
        return 0;

    int matches = 0;

    for (int i = 0; i < p1.size; i++)
    {
        for (int j = 0; j < p1.size; j++)
        {
            if ((p1.cell[i].x == p2.cell[j].x) &&
                (p1.cell[i].y == p2.cell[j].y))
            {
                matches++;
            }
        }
    }
    return (matches == p1.size);
}

int contains(Polyomino p, int x, int y)
{
    for (int i = 0; i < p.size; i++)
        if (p.cell[i].x == x && p.cell[i].y == y)
            return 1;
    return 0;
}

int get_orientations(Polyomino poly, Polyomino **arr)
{
    if (*arr)
        return -1;

    // 4 rotations x 2 flips
    Polyomino p[8];
    p[0] = poly;
    p[1] = zero_bounded(rotated(p[0]));
    p[2] = zero_bounded(rotated(p[1]));
    p[3] = zero_bounded(rotated(p[2]));
    p[4] = zero_bounded(flipped(poly));
    p[5] = zero_bounded(rotated(p[4]));
    p[6] = zero_bounded(rotated(p[5]));
    p[7] = zero_bounded(rotated(p[6]));

    // determine duplicates
    int dup[8] = {0};
    int unique = 8;
    for (int i = 0; i < 8; i++)
    {
        if (dup[i])
            continue;

        for (int j = i + 1; j < 8; j++)
        {
            if (is_equal(p[i], p[j]))
            {
                dup[j] = 1;
                unique--;
            }
        }
    }

    // create output array
    int size = unique;
    if (!(*arr = calloc(size, sizeof(Polyomino))))
    {
        return -2; // bad alloc
    }

    // populate array
    for (int i = 0; i < 8; i++)
    {
        if (!dup[i])
        {
            (*arr)[size - unique] = p[i];
            unique--; // reuse as arr iterator
        }
    }
    return size;
}

void print_poly_bounded(Polyomino p, int x0, int y0, int x1, int y1)
{
    if (p.size <= 0)
        return;

    // print out poly
    for (int y = y0; y <= y1; y++)
    {
        for (int x = x0; x <= x1; x++)
            printf("%c", (contains(p, x, y)) ? '#' : '.');
        printf("\n");
    }
}

void print_poly(Polyomino p)
{
    if (p.size <= 0)
        return;

    // find bounds
    int x0, y0, x1, y1;
    bounds(p, &x0, &y0, &x1, &y1);

    // print out poly
    for (int y = y0; y <= y1; y++)
    {
        for (int x = x0; x <= x1; x++)
            printf("%c", (contains(p, x, y)) ? '#' : ' ');
        printf("\n");
    }
}
