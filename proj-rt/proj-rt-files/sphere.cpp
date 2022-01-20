#include "sphere.h"
#include "ray.h"

// Determine if the ray intersects with the sphere
Hit Sphere::Intersection(const Ray& ray, int part) const
{
    Hit p = {this, 0, part};	//point of intersection
    vec3 ec = ray.endpoint - center;
    vec3 u = ray.direction;
    double a = dot(u,u);
    double b = dot(u+u,ec);
    double c = dot(ec,ec) - radius*radius;
    double d = b*b - 4*a*c;	//the discriminant
	
    if(d > 0){
    	double x1 = (-b - sqrt(d))/2*a;
	double x2 = (-b + sqrt(d))/2*a;
	if(x1 > small_t && x1 < x2 && x2 > small_t){p.dist = x1;}
	else if(x2 > small_t && x1 < small_t){p.dist = x2;}
    }	 	
    return p;
}

vec3 Sphere::Normal(const vec3& point, int part) const
{
    vec3 normal;
    TODO; // compute the normal direction
    return normal;
}

Box Sphere::Bounding_Box(int part) const
{
    Box box;
    TODO; // calculate bounding box
    return box;
}
