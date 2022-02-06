#include "reflective_shader.h"
#include "ray.h"
#include "render_world.h"

vec3 Reflective_Shader::
Shade_Surface(const Ray& ray,const vec3& intersection_point,
    const vec3& normal,int recursion_depth) const
{
    vec3 color;

    if(recursion_depth < world.recursion_depth_limit){
    	vec3 reflection = -2*dot(normal, ray.direction)*normal + ray.direction;
	Ray r(intersection_point, reflection);
	color = (1-reflectivity)*shader->Shade_Surface(ray, intersection_point, normal, recursion_depth) + reflectivity * world.Cast_Ray(r, recursion_depth+1);
    }
    else{     //for recursion end when recursion depth exceed the recursion limit
	    color = (1-reflectivity)*shader->Shade_Surface(ray, intersection_point, normal, recursion_depth);
    }

    return color;
}
