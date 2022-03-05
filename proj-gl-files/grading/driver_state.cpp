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
	state.image_depth[i] = 10;
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
				rasterize_triangle(state, objects[0], objects[1], objects[2]);
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
				rasterize_triangle(state, objects[0], objects[1], objects[2]);
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
				rasterize_triangle(state, objects[0], objects[1], objects[2]);
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
				rasterize_triangle(state, objects[0], objects[1], objects[2]);
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
    if(face==6)
    {
        rasterize_triangle(state, v0, v1, v2);
        return;
    }
    std::cout<<"TODO: implement clipping. (The current code passes the triangle through without clipping them.)"<<std::endl;
    clip_triangle(state, v0, v1, v2,face+1);
}

// Rasterize the triangle defined by the three vertices in the "in" array.  This
// function is responsible for rasterization, interpolation of data to
// fragments, calling the fragment shader, and z-buffering.
void rasterize_triangle(driver_state& state, const data_geometry& v0,
    const data_geometry& v1, const data_geometry& v2)
{
    float x[3];
    float y[3];
    float z[3];

    float width_half = state.image_width / 2;
    float height_half = state.image_height / 2;
    
    x[0] = width_half * (v0.gl_Position[0]/v0.gl_Position[3])  + width_half - 0.5;
    x[1] = width_half * (v1.gl_Position[0]/v1.gl_Position[3])  + width_half - 0.5;
    x[2] = width_half * (v2.gl_Position[0]/v2.gl_Position[3])  + width_half - 0.5;

    y[0] = height_half * (v0.gl_Position[1]/v0.gl_Position[3])  + height_half - 0.5;	   
    y[1] = height_half * (v1.gl_Position[1]/v1.gl_Position[3])  + height_half - 0.5;
    y[2] = height_half * (v2.gl_Position[1]/v2.gl_Position[3])  + height_half - 0.5;

    z[0] = v0.gl_Position[2]/v0.gl_Position[3];
    z[1] = v1.gl_Position[2]/v0.gl_Position[3];
    z[2] = v2.gl_Position[2]/v0.gl_Position[3];

    float x_min = std::max(std::min(std::min(x[0], x[1]), x[2]), float(0.0));
    float x_max = std::min(std::max(std::max(x[0], x[1]), x[2]), float(state.image_width));
    float y_min = std::max(std::min(std::min(y[0], y[1]), y[2]), float(0.0));
    float y_max = std::min(std::max(std::max(y[0], y[1]), y[2]), float(state.image_height));
    
   float* data = new float[MAX_FLOATS_PER_VERTEX];
   data_fragment in;
   in.data = data;
   data_output out;

    float totalArea = (0.5 * ((x[1]*y[2] - x[2]*y[1]) - (x[0]*y[2] - x[2]*y[0]) + (x[0]*y[1] - x[1]*y[0])));

    for(int i = x_min; i < x_max; i++){
	for(int j = y_min; j < y_max; j++){
	    float alpha = (0.5 * ((x[1]*y[2] - x[2]*y[1]) + (j*x[2] - i*y[2]) + (i*y[1] - j*x[1]))) / totalArea;
            float beta = (0.5 * ((i*y[2] - x[2]*j) + (x[2]*y[0] - x[0]*y[2]) + (x[0]*j - y[0]*i))) / totalArea;
            float gamma = (0.5 * ((x[1]*j - i*y[1]) + (i*y[0] - x[0]*j) + (x[0]*y[1] - x[1]*y[0])))/ totalArea;
	    
            if (alpha >= 0 && beta >= 0 && gamma >= 0){
		float z_depth = alpha * z[0] + beta * z[1] + gamma * z[2];
            	if(z_depth < state.image_depth[state.image_width * j + i]){
                	state.image_depth[state.image_width * j + i] = z_depth;
			for(int k = 0; k < state.floats_per_vertex; k++){
                                if(state.interp_rules[k] == interp_type::flat){
                                        in.data[k] = v0.data[k];
                                }
                                else if(state.interp_rules[k] == interp_type::noperspective){
                                        in.data[k] = alpha * v0.data[k] + beta * v1.data[k] + gamma * v2.data[k];
                                }
                                else if(state.interp_rules[k] == interp_type::smooth){
                                        float d = alpha/v0.gl_Position[3] + beta/v1.gl_Position[3] + gamma/v2.gl_Position[3];
                                        in.data[k] = ((alpha/v0.gl_Position[3]/d)*v0.data[k]) + (beta/(d*v1.gl_Position[3])*v1.data[k]) + (gamma/(d*v2.gl_Position[3])*v2.data[k]);
                                }
                        }
			state.fragment_shader(in, out, state.uniform_data);
                	state.image_color[state.image_width * j + i] = make_pixel(out.output_color[0] * 255, out.output_color[1] * 255, out.output_color[2] * 255);
                }
	    }
        }
    }
    delete data;

}


