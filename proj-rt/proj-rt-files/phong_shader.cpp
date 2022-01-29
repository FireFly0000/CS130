#include "light.h"
#include "phong_shader.h"
#include "ray.h"
#include "render_world.h"
#include "object.h"

vec3 Phong_Shader::
Shade_Surface(const Ray& ray,const vec3& intersection_point,
    const vec3& normal,int recursion_depth) const
{
    vec3 color;
    //TODO; //determine the color
    
    //calculating ambient color component Ia = Ra * La ====================
    //Ra = color of the object
    //La = ambient light intensity

    color = (color_ambient * world.ambient_color) * world.ambient_intensity;
    
    // ====================================================================
    //calculating difuse and specular components ==========================
    

    return color;
}
