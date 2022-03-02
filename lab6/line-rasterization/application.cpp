#include "application.h"

#include <iostream>
#include <cassert>
#include <math.h>

#ifndef __APPLE__
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#else
#include <GLUT/glut.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#endif

#define MAX_POINT_SIZE 8

using namespace std;

// convert float color to 32-bit color
inline unsigned int Make_Pixel(const vec3& linecolor)
{
    return ((int)(linecolor[0]*255)<<24)|((int)(linecolor[1]*255)<<16)|((int)(linecolor[2]*255)<<8)|0xff;
}

// set pixel (x,y) in framebuffer to color linecolor, where
// linecolor is a float array of three values between 0 and 1
// which specify the amount of red, green, and blue to mix (e.g.
// RED: (1,0,0) GREEN: (0,1,0) BLUE: (0,0,1)
// YELLOW: (1,1,0) MAGENTA: (1,0,1) CYAN: (0,1,1)
// )
void application::set_pixel(int x, int y, const vec3& linecolor)
{
    unsigned int color=Make_Pixel(linecolor);
    x*=point_size; y*=point_size;
    for(int i=0;i<point_size;i++)
        for(int j=0;j<point_size;j++)
            pixel_data[x+i+(y+j)*width]=color;
}

// This is where the DDA line drawing algorithm is implemented.
// http://en.wikipedia.org/wiki/Digital_differential_analyzer_%28graphics_algorithm%29
void application::draw_line_DDA(int x0, int y0, int x1, int y1, const vec3& linecolor)
{
    float dx = x1 - x0;
    float dy = y1 - y0;

    float m = dy/dx;

    if(abs(m)<1){
        if(x0>x1){
            std::swap(x0,x1);
            std::swap(y0,y1);}
        float cy=y0;
        for(int x = x0; x <= x1; ++x){
            set_pixel(x, round(cy),linecolor);
            cy+=m;}}
    else{
        if(y0>y1){
            std::swap(x0,x1);
            std::swap(y0,y1);}
        float cm=1/m;
        float cx=x0;
        for(int y = y0; y <= y1; ++y){
            set_pixel(round(cx), y,linecolor);
            cx+=cm;}}
}

// This is where the MPA line drawing algorithm is implemented.
// Fix it so that there are no gaps and that it works for all cases while outperforming DDA
// https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm/
void application::Linelow(int x0, int y0, int x1, int y1, const vec3& linecolor){
	int dx = x1 - x0;
	int dy = y1 - y0;
	int yi = 1;
	if(dy < 0){
		yi = -1;
		dy = -dy;
	}
	int D = 2*dy - dx;
	int y = y0;
	for(int i = x0; i <= x1; i++){
		set_pixel(i,y,linecolor);
		if(D>0){
			y = y + yi;
			D = D + (2 * (dy-dx));}
		else{
			D = D + 2*dy;
		}
	}
}
void application::Linehigh(int x0, int y0, int x1, int y1, const vec3& linecolor){
        int dx = x1 - x0;
        int dy = y1 - y0;
        int xi = 1;
        if(dx < 0){
                xi = -1;
                dx = -dx;
        }
        int D = 2*dx - dy;
        int x = x0;
        for(int i = y0; i <= y1; i++){
                set_pixel(x,i,linecolor);
                if(D>0){
                        x = x + xi;
                        D = D + (2 * (dx-dy));}
                else{
                        D = D + 2*dx;
                }
        }
}
void application::draw_line_MPA(int x0, int y0, int x1, int y1, const vec3& linecolor)
{
    // TODO: NOT WORKING CODE (PUT BETTER CODE HERE!!)
    if(abs(y1 - y0) < abs(x1 - x0)){
    	if(x0 > x1){
		Linelow(x1, y1, x0, y0, linecolor);}
	else{
		Linelow(x0, y0, x1, y1, linecolor);}
    }
    else{
    	if(y0 > y1){
		Linehigh(x1, y1, x0, y0, linecolor);}
	else{
		Linehigh(x0, y0, x1, y1, linecolor);}
    }
}

// DDA or MPA algorithms are used based on the mode.
void application::draw_line(const line_segment& line)
{
    if(mode==DDA) draw_line_DDA(line.x0/point_size,line.y0/point_size,line.x1/point_size,line.y1/point_size,line.color);
    else draw_line_MPA(line.x0/point_size,line.y0/point_size,line.x1/point_size,line.y1/point_size,line.color);
}

line_segment application::create_line(int x0,int x1,int y0,int y1,const vec3& linecolor){
    x0=max(0,min(x0,width-MAX_POINT_SIZE+1));
    y0=max(0,min(y0,height-MAX_POINT_SIZE+1));
    x1=max(0,min(x1,width-MAX_POINT_SIZE+1));
    y1=max(0,min(y1,height-MAX_POINT_SIZE+1));
    return line_segment(x0,x1,y0,y1,linecolor);
}



// Adds N number of random lines
void application::add_random_lines(int N)
{
    for(int i=0;i<N;i++){
        vec3 linecolor( ((double) rand() / (RAND_MAX)) + 1, ((double) rand() / (RAND_MAX)) + 1, ((double) rand() / (RAND_MAX)) + 1);
        lines.push_back(line_segment(rand()%(int)(width-MAX_POINT_SIZE+1), rand()%(int)(height-MAX_POINT_SIZE+1), rand()%(int)(width-MAX_POINT_SIZE+1), rand()%(int)(height-MAX_POINT_SIZE+1),linecolor));}
}

// triggered once after the OpenGL context is initialized
void application::init_event()
{

    pixel_data=new unsigned int[width*height];
    srand(5);
    cout << "MOUSE CONTROLS: "<<endl;
    cout << "  Drag left-button to draw a line"<<std::endl;
    cout << "KEYBOARD CONTROLS: "<<endl;
    cout << "  'c': Clear user lines." << endl;
    cout << "  'a': Add 1K random lines." << endl;
    cout << "  'A': Add 1M random lines." << endl;
    cout << "  'm': Switch algorithm." << endl;
    cout << "  ']': increase point_size" << endl;
    cout << "  '[': decrease point_size" << endl;
}

// triggered each time the application needs to redraw
void application::draw_event()
{


    // This code clears the screen pixel data
    for(int i=0;i<width*height;i++) pixel_data[i]=0;

    vec3 white(1.0, 1.0, 1.0);
    vec3 green(0.0, 1.0, 0.0);
    vec3 yellow(1.0, 1.0, 0.0);

    // This code draws a cross mark on the top right side of the screen;
    int centerx=width*3/4;
    int centery=height*3/4;
    int space=(10/point_size)*point_size;
    int lwidth=(60/point_size)*point_size;
    draw_line(line_segment(centerx+space,centery,centerx+lwidth,centery,green));
    draw_line(line_segment(centerx-space,centery,centerx-lwidth,centery,green));
    draw_line(line_segment(centerx,centery+space,centerx,centery+lwidth,green));
    draw_line(line_segment(centerx,centery-space,centerx,centery-lwidth,green));

    // This code draws a circle on the bottom right side of the screen;
    int cuts=16;
    float a=2*M_PI/cuts;
    centery=height/4;
    float radius=70;
    for(int i=0;i<cuts;i++){
        float angle_a=a*i;
        float angle_b=a*(i+1);
        draw_line(line_segment(centerx+cos(angle_a)*radius,centery+sin(angle_a)*radius,centerx+cos(angle_b)*radius,centery+sin(angle_b)*radius,yellow));
    }

    // This code will draw each of the lines and measure the performance
    clock_t begin = clock();
    for (size_t i = 0; i < lines.size(); ++i)
        draw_line(lines[i]);
    clock_t end = clock();
    double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
    if(updated){ updated=false;
        std::cout<<(mode==DDA?"DDA: ":"MPA: ")<<lines.size()<<" lines drawn in "<<elapsed_secs<<" seconds."<<std::endl;}
    
    // This code will draw the data to the screen
    glRasterPos2f(0,0);
    glDrawPixels(width,height,GL_RGBA,GL_UNSIGNED_INT_8_8_8_8,(GLvoid*)pixel_data);
}

// triggered when mouse is clicked
void application::mouse_click_event(int button, int button_state, int x, int y)
{
    // Each time the mouse button is pushed down, add a new
    // line segment to our list.
    if (button == GLUT_LEFT_BUTTON && button_state == GLUT_DOWN) {
        lines.push_back(create_line(x, y, x, y,vec3(1,1,1)));
        button_down = true;
    }
    if (button == GLUT_LEFT_BUTTON && button_state == GLUT_UP) {
        updated=true;
        button_down = false;
    }
    //cout << "button event: " << button << " " << button_state
    //    << " " << x << " " << y << endl;
}

// triggered when mouse button is held down and the mouse is
// moved
void application::mouse_move_event(
    int x, int y
    )
{
    // While the mouse button is down, update the last
    // endpoint in our list to reflect the current location
    // of the mouse.
    lines[lines.size()-1].x1 = max(0,min(x,width-MAX_POINT_SIZE+1));
    lines[lines.size()-1].y1 = max(0,min(y,height-MAX_POINT_SIZE+1));

    //cout << "move event: " << x << " " << y << endl;
}

// triggered when a key is pressed on the keyboard
void application::keyboard_event(unsigned char key, int x, int y)
{
    // clear our stored clicks
    if (key == 'c')
        lines.clear();
    // switch mode
    if (key == 'm'){updated=true;
        mode=1-mode;}
    // add random lines
    if (key == 'a'){updated=true;
        add_random_lines(1000);}
    if (key == 'A'){updated=true;
        add_random_lines(1000000);}
    if (key == ']'){updated=true;
        point_size*=2;
        if(point_size>MAX_POINT_SIZE) point_size=MAX_POINT_SIZE;}
    if (key == '['){updated=true;
        point_size/=2;
        if(point_size<1) point_size=1;}

    //cout << "keyboard event: " << key << " "
    //    << x << " " << y << endl;
}
// triggered when the screen is reshaped
void application::reshape_event(int width_input,int height_input)
{
    delete pixel_data;
    pixel_data=new unsigned int[width_input*height_input];
}
