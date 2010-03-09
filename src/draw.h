#ifndef __DRAW_H__
#define __DRAW_H__

namespace draw 
{
    void draw_square();
    void draw_textured_square(int width, int height);
    void draw_line(float from_x, float from_y, float to_x, float to_y);
    void draw_vertically_flipped_textured_square(float width, float height);
}

#endif // __DRAW_H__
