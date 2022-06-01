#include "light.h"
#include "phong_shader.h"
#include "ray.h"
#include "render_world.h"
#include "object.h"

//comment for github token test
vec3 Phong_Shader::
Shade_Surface(const Ray& ray,const vec3& intersection_point,
    const vec3& normal,int recursion_depth) const
{
    vec3 color;
    vec3 ambient, diffuse, specular;
    //TODO; //determine the color
    
    //calculating ambient color component Ia = Ra * La ====================
    //Ra = color of the object
    //La = ambient light intensity

    ambient = (color_ambient * world.ambient_color) * world.ambient_intensity;
    color = ambient;
    // ====================================================================
    //calculating difuse and specular components ==========================
    
    for(unsigned int i = 0; i < world.lights.size(); i++){
   	vec3 light = world.lights.at(i)->position - intersection_point;		
	vec3 l = light.normalized();		// l is normalized light direction
	vec3 n = normal.normalized();		// n is normalized normal vector at intersection point
	Ray shadow(intersection_point, l);
	Hit shadow_obj = world.Closest_Intersection(shadow);

	if(shadow_obj.object != NULL && shadow_obj.dist < light.magnitude() && world.enable_shadows && shadow_obj.dist > small_t){
		color = color;
	}	
	else if(shadow_obj.object == NULL || !world.enable_shadows || shadow_obj.dist > light.magnitude() || shadow_obj.dist <= small_t){
		//diffuse component
		vec3 Ld = world.lights.at(i)->Emitted_Light(world.lights.at(i)->position - intersection_point);
		diffuse = Ld * color_diffuse * std::max(dot(n,l),0.0);
		//=======================
		//specular component
		vec3 r = 2*(dot(n,l))*n - l; 		    // r is reflection of light ray about the normal at intersection point
		vec3 v = (ray.direction).normalized() * -1; // negative of the view ray direction at the object
		specular = Ld * color_specular * pow(std::max(dot(r,v),0.0), specular_power);  

		color = color + diffuse + specular;
	}
    }
    return color;
}
