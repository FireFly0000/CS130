#include "plane.h"
#include "ray.h"
#include <cfloat>
#include <limits>

// Intersect with the half space defined by the plane.  The plane's normal
// points outside.  If the ray starts on the "inside" side of the plane, be sure
// to record a hit with t=0 as the first entry in hits.
Hit Plane::Intersection(const Ray& ray, int part) const
{
    Hit p = {this, 0, part};	//point of intersection
    vec3 e = ray.endpoint;
    vec3 u = ray.direction;
    vec3 xe = x1 - e;
    if(dot(u,normal) == 0){return p;}
    else if(dot(u,normal) !=0){
    	double a = dot(xe, normal);
	double b = dot(u,normal);
	if(a/b > small_t){p.dist = a/b;}
    }
    return p;
}

vec3 Plane::Normal(const vec3& point, int part) const
{
    return normal;
}

// There is not a good answer for the bounding box of an infinite object.
// The safe thing to do is to return a box that contains everything.
Box Plane::Bounding_Box(int part) const
{
    Box b;
    b.hi.fill(std::numeric_limits<double>::max());
    b.lo=-b.hi;
    return b;
}
