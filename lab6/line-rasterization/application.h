#ifndef APPLICATION_H
#define APPLICATION_H

#include <vector>

#include "gl_viewer.h"
#include "vec.h"

class line_segment
{
public:
    // line_segment(int x0, int y0, int x1, int y1,const vec3& color=vec3(0,0,0))
    line_segment(int x0, int y0, int x1, int y1,const vec3& color)
        : x0(x0), y0(y0), x1(x1), y1(y1),color(color)
    {}

    int x0, y0, x1, y1;
    vec3 color;
};

class application : public gl_viewer
{
    enum{DDA=0,MPA};
public:
    application() :button_down(false),mode(MPA),updated(false),pixel_data(0),point_size(1) {}
    virtual ~application() {delete pixel_data;}

    void init_event();
    void draw_event();
    void mouse_click_event(int button, int button_state, int x, int y);
    void mouse_move_event(int x, int y);
    void keyboard_event(unsigned char key, int x, int y);
    virtual void reshape_event(int width_input,int height_input);

private:

    void set_pixel(int x, int y, const vec3& linecolor=vec3(1,1,1));
    void draw_line(const line_segment& line);
    line_segment create_line(int x0, int y0, int x1, int y1, const vec3& linecolor);
    void draw_line_DDA(int x0, int y0, int x1, int y1, const vec3& linecolor);
    void draw_line_MPA(int x0, int y0, int x1, int y1, const vec3& linecolor);
    void add_random_lines(int N);
    void Linelow(int x0, int y0, int x1, int y1, const vec3& linecolor);
    void Linehigh(int x0, int y0, int x1, int y1, const vec3& linecolor);

    std::vector<line_segment> lines; // store the line segments
    bool button_down;
    int mode;
    bool updated;
    unsigned int* pixel_data;
    int point_size;
};

#endif
