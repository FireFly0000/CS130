#include "driver_state.h"
#include <cstring>

driver_state::driver_state()
{
}

driver_state::~driver_state()
{
    delete [] image_color;
    delete [] image_depth;
}

// This function should allocate and initialize the arrays that store color and
// depth.  This is not done during the constructor since the width and height
// are not known when this class is constructed.
void initialize_render(driver_state& state, int width, int height)
{
    state.image_width=width;
    state.image_height=height;
    state.image_color= new pixel[width*height];
    state.image_depth= new float[width*height];
    
    pixel black = make_pixel(0,0,0);

    for(int i = 0; i < width*height; i++){
	state.image_color[i] = black;
	state.image_depth[i] = 100;
    }
}

// This function will be called to render the data that has been stored in this class.
// Valid values of type are:
//   render_type::triangle - Each group of three vertices corresponds to a triangle.
//   render_type::indexed -  Each group of three indices in index_data corresponds
//                           to a triangle.  These numbers are indices into vertex_data.
//   render_type::fan -      The vertices are to be interpreted as a triangle fan.
//   render_type::strip -    The vertices are to be interpreted as a triangle strip.
void render(driver_state& state, render_type type)
{
	data_geometry objects[3];
	data_vertex temp[3];

	switch(type){
		case render_type::triangle:{
			int x = 0;
			for(int i = 0; i < state.num_vertices/3; i++){
				for(int j = 0; j < 3; j++){
					temp[j].data = &state.vertex_data[x];
					objects[j].data = temp[j].data;
					state.vertex_shader(temp[j], objects[j], state.uniform_data);
					x = x+state.floats_per_vertex;					
				}
				clip_triangle(state, objects[0], objects[1], objects[2], 4);
			}
			break;
		}
		case render_type::indexed:{
			for(int i = 0; i < state.num_triangles*3; i = i+3){
				for(int j = 0; j<3; j++){
					temp[j].data = &state.vertex_data[state.index_data[i+j] * state.floats_per_vertex];
					objects[j].data = temp[j].data;
					state.vertex_shader(temp[j], objects[j], state.uniform_data);
				}
				clip_triangle(state, objects[0], objects[1], objects[2], 4);
			}
			break;
		}
		case render_type::fan:{
			for(int i = 0; i < state.num_vertices; i++){
				for(int j = 0; j < 3; j++){
					if(j == 0){
						temp[j].data = &state.vertex_data[0];
					}
					else{
						temp[j].data = &state.vertex_data[(i+j) * state.floats_per_vertex];
					}
					objects[j].data = temp[j].data;
					state.vertex_shader(temp[j], objects[j], state.uniform_data);
				}
				clip_triangle(state, objects[0], objects[1], objects[2], 4);
			}
			break;
		}
		case render_type::strip:{
			for(int i = 0; i < state.num_vertices-2; i++){
				for(int j = 0; j<3; j++){
					temp[j].data = &state.vertex_data[(i+j) * state.floats_per_vertex];
					objects[j].data = temp[j].data;
					state.vertex_shader(temp[j], objects[j], state.uniform_data);
				}
				clip_triangle(state, objects[0], objects[1], objects[2], 4);
			}
			break;
		}
		default: {break;}
	}

}


// This function clips a triangle (defined by the three vertices in the "in" array).
// It will be called recursively, once for each clipping face (face=0, 1, ..., 5) to
// clip against each of the clipping faces in turn.  When face=6, clip_triangle should
// simply pass the call on to rasterize_triangle.
void clip_triangle(driver_state& state, const data_geometry& v0,
    const data_geometry& v1, const data_geometry& v2,int face)
{
    vec4 a = v0.gl_Position;
    vec4 b = v1.gl_Position;
    vec4 c = v2.gl_Position;
    if(face==6){
        rasterize_triangle(state, v0, v1, v2);
        return;
    }
    else if(face == 0){ //right plane
    	if(a[0] <= a[3] && b[0] <= b[3] && c[0] <= c[3]){ //all three points are inside of plane
		clip_triangle(state, v0, v1, v2,face+1);
	}
	else if(a[0] > a[3] && b[0] > b[3] && c[0] > c[3]){ //all three points are outside of plane, do nothing
		return;
	}
	else if( (a[0] > a[3] && b[0] <= b[3] && c[0] <= c[3]) || //two points are inside of plane
		 (a[0] <= a[3] && b[0] > b[3] && c[0] <= c[3]) ||
		 (a[0] <= a[3] && b[0] <= b[3] && c[0] > c[3])){
			vec4 in1, in2, out;
			data_geometry A, B, C;		//A and B are always the two inside, C is always the one outside
			if(b[0] <= b[3] && c[0] <= c[3]){ in1 = b; in2 = c; out = a; A = v1; B = v2; C = v0;}
			else if(a[0] <= a[3] && c[0] <= c[3]){ in1 = a; in2 = c; out = b; A = v0; B = v2; C = v1;}
			else if(a[0] <= a[3] && b[0] <= b[3]){ in1 = a; in2 = b; out = c; A = v0; B = v1; C = v2;}
			float alpha1 = (out[3] - out[0])/(in1[0] - in1[3] + out[3] - out[0]);
			float alpha2 = (out[3] - out[0])/(in2[0] - in2[3] + out[3] - out[0]);
			vec4 intersect_1 = alpha1*in1 +(1-alpha1)*out;
			vec4 intersect_2 = alpha2*in2 +(1-alpha2)*out;
		        
			C.gl_Position = intersect_1;
			clip_triangle(state, A, B, C, face + 1);

			A.gl_Position = intersect_2;
			clip_triangle(state, A, B, C, face + 1);	
		 
	}
	else if( (a[0] <= a[3] && b[0] > b[3] && c[0] > c[3]) || //one point is inside of plane
		 (a[0] > a[3] && b[0] <= b[3] && c[0] > c[3]) ||
	         (a[0] > a[3] && b[0] > b[3] && c[0] <= c[3])) {
			vec4 in, out1, out2;
                        data_geometry A, B, C;            //A is always the one inside, B and C are always the one outside
                        if(a[0] <= a[3]){ in = a; out1 = b; out2 = c; A = v0; B = v1; C = v2;}
                        else if(b[0] <= b[3]){ in = b; out1 = a; out2 = c; A = v1; B = v0; C = v2;}
                        else if(c[0] <= c[3]){ in = c; out1 = a; out2 = b; A = v2; B = v0; C = v1;}
			float alpha1 = (out1[3] - out1[0])/(in[0] - in[3] + out1[3] - out1[0]);
                        float alpha2 = (out2[3] - out2[0])/(in[0] - in[3] + out2[3] - out2[0]);
			vec4 intersect_1 = alpha1*in +(1-alpha1)*out1;
                        vec4 intersect_2 = alpha2*in +(1-alpha2)*out2;

                        B.gl_Position = intersect_1;
			C.gl_Position = intersect_2;
                        clip_triangle(state, A, B, C, face + 1);

	}	 
    }
    else if(face == 1){ //left plane
	if(a[0] >= -a[3] && b[0] >= -b[3] && c[0] >= -c[3]){ //all three points are inside of plane
                clip_triangle(state, v0, v1, v2,face+1);
        }
        else if(a[0] < -a[3] && b[0] < -b[3] && c[0] < -c[3]){ //all three points are outside of plane, do nothing
                return;
        }
        else if( (a[0] < -a[3] && b[0] >= -b[3] && c[0] >= -c[3]) || //two points are inside of plane
                 (a[0] >= -a[3] && b[0] < -b[3] && c[0] >= -c[3]) ||
                 (a[0] >= -a[3] && b[0] >= -b[3] && c[0] < -c[3])){
                        vec4 in1, in2, out;
                        data_geometry A, B, C;          //A and B are always the two inside, C is always the one outside
                        if(b[0] >= -b[3] && c[0] >= -c[3]){ in1 = b; in2 = c; out = a; A = v1; B = v2; C = v0;}
                        else if(a[0] >= -a[3] && c[0] >= -c[3]){ in1 = a; in2 = c; out = b; A = v0; B = v2; C = v1;}
                        else if(a[0] >= -a[3] && b[0] >= -b[3]){ in1 = a; in2 = b; out = c; A = v0; B = v1; C = v2;}
                        float alpha1 = (-out[3] - out[0])/(in1[0] + in1[3] - out[3] - out[0]);
                        float alpha2 = (-out[3] - out[0])/(in2[0] + in2[3] - out[3] - out[0]);
                        vec4 intersect_1 = alpha1*in1 +(1-alpha1)*out;
                        vec4 intersect_2 = alpha2*in2 +(1-alpha2)*out;

                        C.gl_Position = intersect_1;
                        clip_triangle(state, A, B, C, face + 1);

                        A.gl_Position = intersect_2;
                        clip_triangle(state, A, B, C, face + 1);

        }
        else if( (a[0] >= -a[3] && b[0] < -b[3] && c[0] < -c[3]) || //one point is inside of plane
                 (a[0] < -a[3] && b[0] >= -b[3] && c[0] < -c[3]) ||
                 (a[0] < -a[3] && b[0] < -b[3] && c[0] >= -c[3])) {
                        vec4 in, out1, out2;
                        data_geometry A, B, C;            //A is always the one inside, B and C are always the one outside
                        if(a[0] >= -a[3]){ in = a; out1 = b; out2 = c; A = v0; B = v1; C = v2;}
                        else if(b[0] >= -b[3]){ in = b; out1 = a; out2 = c; A = v1; B = v0; C = v2;}
                        else if(c[0] >= -c[3]){ in = c; out1 = a; out2 = b; A = v2; B = v0; C = v1;}
                        float alpha1 = (-out1[3] - out1[0])/(in[0] + in[3] - out1[3] - out1[0]);
                        float alpha2 = (-out2[3] - out2[0])/(in[0] + in[3] - out2[3] - out2[0]);
                        vec4 intersect_1 = alpha1*in +(1-alpha1)*out1;
                        vec4 intersect_2 = alpha2*in +(1-alpha2)*out2;

                        B.gl_Position = intersect_1;
                        C.gl_Position = intersect_2;
                        clip_triangle(state, A, B, C, face + 1);

	}
    }
   else if(face == 2){ //top plane
	if(a[1] <= a[3] && b[1] <= b[3] && c[1] <= c[3]){ //all three points are inside of plane
                clip_triangle(state, v0, v1, v2,face+1);
        }
        else if(a[1] > a[3] && b[1] > b[3] && c[1] > c[3]){ //all three points are outside of plane, do nothing
                return;
        }
        else if( (a[1] > a[3] && b[1] <= b[3] && c[1] <= c[3]) || //two points are inside of plane
                 (a[1] <= a[3] && b[1] > b[3] && c[1] <= c[3]) ||
                 (a[1] <= a[3] && b[1] <= b[3] && c[1] > c[3])){
                        vec4 in1, in2, out;
                        data_geometry A, B, C;          //A and B are always the two inside, C is always the one outside
                        if(b[1] <= b[3] && c[1] <= c[3]){ in1 = b; in2 = c; out = a; A = v1; B = v2; C = v0;}
                        else if(a[1] <= a[3] && c[1] <= c[3]){ in1 = a; in2 = c; out = b; A = v0; B = v2; C = v1;}
                        else if(a[1] <= a[3] && b[1] <= b[3]){ in1 = a; in2 = b; out = c; A = v0; B = v1; C = v2;}
                        float alpha1 = (out[3] - out[1])/(in1[1] - in1[3] + out[3] - out[1]);
                        float alpha2 = (out[3] - out[1])/(in2[1] - in2[3] + out[3] - out[1]);
                        vec4 intersect_1 = alpha1*in1 +(1-alpha1)*out;
                        vec4 intersect_2 = alpha2*in2 +(1-alpha2)*out;

                        C.gl_Position = intersect_1;
                        clip_triangle(state, A, B, C, face + 1);

                        A.gl_Position = intersect_2;
                        clip_triangle(state, A, B, C, face + 1);

        }
        else if( (a[1] <= a[3] && b[1] > b[3] && c[1] > c[3]) || //one point is inside of plane
                 (a[1] > a[3] && b[1] <= b[3] && c[1] > c[3]) ||
                 (a[1] > a[3] && b[1] > b[3] && c[1] <= c[3])) {
                        vec4 in, out1, out2;
                        data_geometry A, B, C;            //A is always the one inside, B and C are always the one outside
                        if(a[1] <= a[3]){ in = a; out1 = b; out2 = c; A = v0; B = v1; C = v2;}
                        else if(b[1] <= b[3]){ in = b; out1 = a; out2 = c; A = v1; B = v0; C = v2;}
                        else if(c[1] <= c[3]){ in = c; out1 = a; out2 = b; A = v2; B = v0; C = v1;}
                        float alpha1 = (out1[3] - out1[1])/(in[1] - in[3] + out1[3] - out1[1]);
                        float alpha2 = (out2[3] - out2[1])/(in[1] - in[3] + out2[3] - out2[1]);
                        vec4 intersect_1 = alpha1*in +(1-alpha1)*out1;
                        vec4 intersect_2 = alpha2*in +(1-alpha2)*out2;

                        B.gl_Position = intersect_1;
                        C.gl_Position = intersect_2;
                        clip_triangle(state, A, B, C, face + 1);
	}
   }
   else if(face == 3){ //bottom plane
	if(a[1] >= -a[3] && b[1] >= -b[3] && c[1] >= -c[3]){ //all three points are inside of plane
                clip_triangle(state, v0, v1, v2,face+1);
        }
        else if(a[1] < -a[3] && b[1] < -b[3] && c[1] < -c[3]){ //all three points are outside of plane, do nothing
                return;
        }
        else if( (a[1] < -a[3] && b[1] >= -b[3] && c[1] >= -c[3]) || //two points are inside of plane
                 (a[1] >= -a[3] && b[1] < -b[3] && c[1] >= -c[3]) ||
                 (a[1] >= -a[3] && b[1] >= -b[3] && c[1] < -c[3])){
                        vec4 in1, in2, out;
                        data_geometry A, B, C;          //A and B are always the two inside, C is always the one outside
                        if(b[1] >= -b[3] && c[1] >= -c[3]){ in1 = b; in2 = c; out = a; A = v1; B = v2; C = v0;}
                        else if(a[1] >= -a[3] && c[1] >= -c[3]){ in1 = a; in2 = c; out = b; A = v0; B = v2; C = v1;}
                        else if(a[1] >= -a[3] && b[1] >= -b[3]){ in1 = a; in2 = b; out = c; A = v0; B = v1; C = v2;}
                        float alpha1 = (-out[3] - out[1])/(in1[1] + in1[3] - out[3] - out[1]);
                        float alpha2 = (-out[3] - out[1])/(in2[1] + in2[3] - out[3] - out[1]);
                        vec4 intersect_1 = alpha1*in1 +(1-alpha1)*out;
                        vec4 intersect_2 = alpha2*in2 +(1-alpha2)*out;

                        C.gl_Position = intersect_1;
                        clip_triangle(state, A, B, C, face + 1);

                        A.gl_Position = intersect_2;
                        clip_triangle(state, A, B, C, face + 1);

        }
        else if( (a[1] >= -a[3] && b[1] < -b[3] && c[1] < -c[3]) || //one point is inside of plane
                 (a[1] < -a[3] && b[1] >= -b[3] && c[1] < -c[3]) ||
                 (a[1] < -a[3] && b[1] < -b[3] && c[1] >= -c[3])) {
                        vec4 in, out1, out2;
                        data_geometry A, B, C;            //A is always the one inside, B and C are always the one outside
                        if(a[1] >= -a[3]){ in = a; out1 = b; out2 = c; A = v0; B = v1; C = v2;}
                        else if(b[1] >= -b[3]){ in = b; out1 = a; out2 = c; A = v1; B = v0; C = v2;}
                        else if(c[1] >= -c[3]){ in = c; out1 = a; out2 = b; A = v2; B = v0; C = v1;}
                        float alpha1 = (-out1[3] - out1[1])/(in[1] + in[3] - out1[3] - out1[1]);
                        float alpha2 = (-out2[3] - out2[1])/(in[1] + in[3] - out2[3] - out2[1]);
                        vec4 intersect_1 = alpha1*in +(1-alpha1)*out1;
                        vec4 intersect_2 = alpha2*in +(1-alpha2)*out2;

                        B.gl_Position = intersect_1;
                        C.gl_Position = intersect_2;
                        clip_triangle(state, A, B, C, face + 1);

        }	   
   }
   else if(face == 4){ //far plane
   	if(a[2] <= a[3] && b[2] <= b[3] && c[2] <= c[3]){ //all three points are inside of plane
                clip_triangle(state, v0, v1, v2,face+1);
        }
        else if(a[2] > a[3] && b[2] > b[3] && c[2] > c[3]){ //all three points are outside of plane, do nothing
                return;
        }
        else if( (a[2] > a[3] && b[2] <= b[3] && c[2] <= c[3]) || //two points are inside of plane
                 (a[2] <= a[3] && b[2] > b[3] && c[2] <= c[3]) ||
                 (a[2] <= a[3] && b[2] <= b[3] && c[2] > c[3])){
                        vec4 in1, in2, out;
                        data_geometry A, B, C;          //A and B are always the two inside, C is always the one outside
                        if(b[2] <= b[3] && c[2] <= c[3]){ in1 = b; in2 = c; out = a; A = v1; B = v2; C = v0;}
                        else if(a[2] <= a[3] && c[2] <= c[3]){ in1 = a; in2 = c; out = b; A = v0; B = v2; C = v1;}
                        else if(a[2] <= a[3] && b[2] <= b[3]){ in1 = a; in2 = b; out = c; A = v0; B = v1; C = v2;}
                        float alpha1 = (out[3] - out[2])/(in1[2] - in1[3] + out[3] - out[2]);
                        float alpha2 = (out[3] - out[2])/(in2[2] - in2[3] + out[3] - out[2]);
                        vec4 intersect_1 = alpha1*in1 +(1-alpha1)*out;
                        vec4 intersect_2 = alpha2*in2 +(1-alpha2)*out;

                        C.gl_Position = intersect_1;
                        clip_triangle(state, A, B, C, face + 1);

                        A.gl_Position = intersect_2;
                        clip_triangle(state, A, B, C, face + 1);

        }
        else if( (a[2] <= a[3] && b[2] > b[3] && c[2] > c[3]) || //one point is inside of plane
                 (a[2] > a[3] && b[2] <= b[3] && c[2] > c[3]) ||
                 (a[2] > a[3] && b[2] > b[3] && c[2] <= c[3])) {
                        vec4 in, out1, out2;
                        data_geometry A, B, C;            //A is always the one inside, B and C are always the one outside
                        if(a[2] <= a[3]){ in = a; out1 = b; out2 = c; A = v0; B = v1; C = v2;}
                        else if(b[2] <= b[3]){ in = b; out1 = a; out2 = c; A = v1; B = v0; C = v2;}
                        else if(c[2] <= c[3]){ in = c; out1 = a; out2 = b; A = v2; B = v0; C = v1;}
                        float alpha1 = (out1[3] - out1[2])/(in[2] - in[3] + out1[3] - out1[2]);
                        float alpha2 = (out2[3] - out2[2])/(in[2] - in[3] + out2[3] - out2[2]);
                        vec4 intersect_1 = alpha1*in +(1-alpha1)*out1;
                        vec4 intersect_2 = alpha2*in +(1-alpha2)*out2;

                        B.gl_Position = intersect_1;
                        C.gl_Position = intersect_2;
                        clip_triangle(state, A, B, C, face + 1);

        }	
   }
   else if(face == 5){ //near plane
   	if(a[2] >= -a[3] && b[2] >= -b[3] && c[2] >= -c[3]){ //all three points are inside of plane
                clip_triangle(state, v0, v1, v2,face+1);
        }
        else if(a[2] < -a[3] && b[2] < -b[3] && c[2] < -c[3]){ //all three points are outside of plane, do nothing
                return;
        }
        else if( (a[2] < -a[3] && b[2] >= -b[3] && c[2] >= -c[3]) || //two points are inside of plane
                 (a[2] >= -a[3] && b[2] < -b[3] && c[2] >= -c[3]) ||
                 (a[2] >= -a[3] && b[2] >= -b[3] && c[2] < -c[3])){
                        vec4 in1, in2, out;
                        data_geometry A, B, C;          //A and B are always the two inside, C is always the one outside
                        if(b[2] >= -b[3] && c[2] >= -c[3]){ in1 = b; in2 = c; out = a; A = v1; B = v2; C = v0;}
                        else if(a[2] >= -a[3] && c[2] >= -c[3]){ in1 = a; in2 = c; out = b; A = v0; B = v2; C = v1;}
                        else if(a[2] >= -a[3] && b[2] >= -b[3]){ in1 = a; in2 = b; out = c; A = v0; B = v1; C = v2;}
                        float alpha1 = (-out[3] - out[2])/(in1[2] + in1[3] - out[3] - out[2]);
                        float alpha2 = (-out[3] - out[2])/(in2[2] + in2[3] - out[3] - out[2]);
                        vec4 intersect_1 = alpha1*in1 +(1-alpha1)*out;
                        vec4 intersect_2 = alpha2*in2 +(1-alpha2)*out;

                        C.gl_Position = intersect_1;
                        clip_triangle(state, A, B, C, face + 1);

                        A.gl_Position = intersect_2;
                        clip_triangle(state, A, B, C, face + 1);

        }
        else if( (a[2] >= -a[3] && b[2] < -b[3] && c[2] < -c[3]) || //one point is inside of plane
                 (a[2] < -a[3] && b[2] >= -b[3] && c[2] < -c[3]) ||
                 (a[2] < -a[3] && b[2] < -b[3] && c[2] >= -c[3])) {
                        vec4 in, out1, out2;
                        data_geometry A, B, C;            //A is always the one inside, B and C are always the one outside
                        if(a[2] >= -a[3]){ in = a; out1 = b; out2 = c; A = v0; B = v1; C = v2;}
                        else if(b[2] >= -b[3]){ in = b; out1 = a; out2 = c; A = v1; B = v0; C = v2;}
                        else if(c[2] >= -c[3]){ in = c; out1 = a; out2 = b; A = v2; B = v0; C = v1;}
                        float alpha1 = (-out1[3] - out1[2])/(in[2] + in[3] - out1[3] - out1[2]);
                        float alpha2 = (-out2[3] - out2[2])/(in[2] + in[3] - out2[3] - out2[2]);
                        vec4 intersect_1 = alpha1*in +(1-alpha1)*out1;
                        vec4 intersect_2 = alpha2*in +(1-alpha2)*out2;

                        B.gl_Position = intersect_1;
                        C.gl_Position = intersect_2;
                        clip_triangle(state, A, B, C, face + 1);

        }				
   } 
}

// Rasterize the triangle defined by the three vertices in the "in" array.  This
// function is responsible for rasterization, interpolation of data to
// fragments, calling the fragment shader, and z-buffering.
void rasterize_triangle(driver_state& state, const data_geometry& v0,
    const data_geometry& v1, const data_geometry& v2)
{
    const data_geometry* in[3];
    in[0] = &v0;
    in[1] = &v1;
    in[2] = &v2;

    float x[3];
    float y[3];
    float z[3];

    float width_half = state.image_width / 2;
    float height_half = state.image_height / 2;
    
    for(int i = 0; i < 3; i++){
    	x[i] = width_half * ((*in)[i].gl_Position[0])/((*in)[i].gl_Position[3])  + width_half - 0.5;
    	y[i] = height_half * ((*in)[i].gl_Position[1])/((*in)[i].gl_Position[3])  + height_half - 0.5;	   
    	z[i] = (*in)[i].gl_Position[2]/(*in)[i].gl_Position[3];
   }

    float x_min = std::max(std::min(std::min(x[0], x[1]), x[2]), float(0.0));
    float x_max = std::min(std::max(std::max(x[0], x[1]), x[2]), float(state.image_width));
    float y_min = std::max(std::min(std::min(y[0], y[1]), y[2]), float(0.0));
    float y_max = std::min(std::max(std::max(y[0], y[1]), y[2]), float(state.image_height));
    
   float* data = new float[MAX_FLOATS_PER_VERTEX];
   data_fragment input;
   input.data = data;
   data_output out;

    float totalArea = (0.5 * ((x[1]*y[2] - x[2]*y[1]) - (x[0]*y[2] - x[2]*y[0]) + (x[0]*y[1] - x[1]*y[0])));

    for(int i = x_min+1; i < x_max; i++){
	for(int j = y_min+1; j < y_max; j++){
	    float alpha = (0.5 * ((x[1]*y[2] - x[2]*y[1]) + (j*x[2] - i*y[2]) + (i*y[1] - j*x[1]))) / totalArea;
            float beta = (0.5 * ((i*y[2] - x[2]*j) + (x[2]*y[0] - x[0]*y[2]) + (x[0]*j - y[0]*i))) / totalArea;
            float gamma = (0.5 * ((x[1]*j - i*y[1]) + (i*y[0] - x[0]*j) + (x[0]*y[1] - x[1]*y[0])))/ totalArea;
	    
            if (alpha >= 0 && beta >= 0 && gamma >= 0){
		float z_depth = alpha * z[0] + beta * z[1] + gamma * z[2];
            	if(z_depth < state.image_depth[state.image_width * j + i]){
                	state.image_depth[state.image_width * j + i] = z_depth;
			for(int k = 0; k < state.floats_per_vertex; k++){
				if(state.interp_rules[k] == interp_type::flat){
                                	input.data[k] = v0.data[k];
                               	}
				else if(state.interp_rules[k] == interp_type::smooth){
                                	float d = alpha/v0.gl_Position[3] + beta/v1.gl_Position[3] + gamma/v2.gl_Position[3];
                                	input.data[k] = ((alpha/v0.gl_Position[3]/d)*v0.data[k]) + (beta/(d*v1.gl_Position[3])*v1.data[k]) + (gamma/(d*v2.gl_Position[3])*v2.data[k]);
                                }
				else if(state.interp_rules[k] == interp_type::noperspective){
                                	input.data[k] = alpha * v0.data[k] + beta * v1.data[k] + gamma * v2.data[k];
				}
                        }
			state.fragment_shader(input, out, state.uniform_data);
                	state.image_color[state.image_width * j + i] = make_pixel(out.output_color[0] * 255, out.output_color[1] * 255, out.output_color[2] * 255);
                }
	    }
      	}
    }
    delete data;

}


