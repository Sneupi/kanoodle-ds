
#include "graphics.h"

void set_poly_sprite(PolySprite *p, int x, int y)
{
    p->x = x;
    p->y = y;
    for (int i = 0; i < p->size; i++)
    {
        Sprite *s = &(p->sprites[i]);
        int sprite_offset_x = x + (s->w * p->poly.cell[i].x);
        int sprite_offset_y = y + (s->h * p->poly.cell[i].y);
        set_sprite(s, sprite_offset_x, sprite_offset_y);
    }
}

void shift_poly_sprite(PolySprite *p, int dx, int dy)
{
    set_poly_sprite(p, (p->x + dx), (p->y + dy));
}

PolySprite init_poly_sprite(Polyomino poly, SpriteGfx gfx)
{
    PolySprite p = {0};
    p.poly = zero_bounded(poly);
    p.size = poly.size;
    p.x = 0;
    p.y = 0;

    for (int i = 0; i < p.size; i++) {
        Sprite *s = &p.sprites[i];
        *s = init_sprite(gfx);

        // these ensure during relative shifts of cells, 
        // they all appear nicely connected
        s->h = POLYBEAD_HEIGHT;
        s->w = POLYBEAD_WIDTH;
    }

    set_poly_sprite(&p, 0, 0);
    return p;
}
