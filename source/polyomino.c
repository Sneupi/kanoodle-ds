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

int get_orientations(Polyomino poly, Polyomino arr[8])
{
    // 4 rotations x 2 flips
    Polyomino o[8];
    o[0] = zero_bounded(poly);
    o[1] = zero_bounded(rotated(o[0]));
    o[2] = zero_bounded(rotated(o[1]));
    o[3] = zero_bounded(rotated(o[2]));
    o[4] = zero_bounded(flipped(poly));
    o[5] = zero_bounded(rotated(o[4]));
    o[6] = zero_bounded(rotated(o[5]));
    o[7] = zero_bounded(rotated(o[6]));

    arr[0] = o[0];
    int size = 1;

    for (int i = 1; i < 8; i++)
    {
        // determine if duplicate
        int dup = 0;
        for (int j = 0; j < size; j++)
        {
            if (is_equal(o[i], arr[j]))
            {
                dup = 1;
                break;
            }
        }
        // add nonduplicates
        if (!dup)
        {
            arr[size] = o[i];
            size++;
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
