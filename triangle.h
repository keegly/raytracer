#ifndef TRIANGLEH
#define TRIANGLEH
#include "hitable.h"

class triangle : public hitable {
public:
	//     c
	//	  / \
	//   /   \
	//  /     \
	// /       \
	//a---------b
	//
	triangle() {}
	triangle(vec3 _a, vec3 _b, vec3 _c, material *t) : a(_a), b(_b), c(_c), mat(t) {}
	virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const;
	virtual bool bounding_box(float t0, float t1, aabb& box) const;
	
	// Vertices
	vec3 a, b, c;
	material *mat;
};

bool triangle::hit(const ray& r, float t_min, float t_max, hit_record& rec) const {
	//extract vectors from vertices of triangle
	vec3 v0 = b - a;
	vec3 v1 = c - a;
	vec3 normal = cross(v0, v1);

	// this tells us how steep of an angle our ray is approaching the front of the triangle
	//float denom = dot(normal, r.direction());
	// reject rays that wont hit triangle
	//if (!(denom < 0.0f))
	//	return false;
	// wtf is this wizardry?!
	
	vec3 dir = r.direction();
	vec3 pvec = cross(dir, v1);
	float determinant = dot(pvec, v0);
	float invDet = 1 / determinant;
	vec3 tvec = r.origin() - a;
	vec3 qvec = cross(tvec, v0);
	float t_2 = dot(v1, qvec) * invDet;
	if (t_2 < 0) return false;
	float u_2 = dot(tvec, pvec) *invDet;
	if (u_2 < 0 || u_2 > 1) return false;
	float v_2 = dot(r.direction(), qvec) * invDet;
	if (v_2 < 0 || u_2 + v_2 > 1) return false;
	vec3 phit_2 = r.point_at_parameter(t_2);
	

	//float d = dot(normal, a); // Ax + By + Cz = d
	//float t = (dot(normal, r.origin()) + d) / denom;
	//float t = d - dot(normal, r.origin()) / denom;

	// triangle is behind the ray or too far away(?)
	/*if (t < t_min || t > t_max)
		return false;
	
	// same as r.point_at_parameter(t);
	// vec3 phit = r.origin() + t  * r.direction();
	vec3 phit = r.point_at_parameter(t);
	
	vec3 v2 = phit - a;

	// barycentric coords
	float d00 = dot(v0, v0);
	float d01 = dot(v0, v1);
	float d11 = dot(v1, v1);
	float d20 = dot(v2, v0);
	float d21 = dot(v2, v1);
	denom = d00 * d11 - d01 * d01;
	float v = (d11 * d20 - d01 * d21) / denom;
	float w = (d00 * d21 - d01 * d20) / denom;
	float u = 1.0f - v - w;

	// check if hit point is inside triangle
	if (u < 0 || v < 0 || u + v > 1)
		return false;
	*/
	rec.u = u_2;
	rec.v = v_2;
	rec.t = t_2;
	rec.p = phit_2;
	rec.mat_ptr = mat;
	rec.normal = normal;
	return true;
}

bool triangle::bounding_box(float t0, float t1, aabb& box) const {
	/*
	x0 y1			x1 y1
	a.x, c.y     b.x, c.y
	    *	 /\     * 
		    /  \
		   /    \
		  /      \
		* --------- *
	a.x, a.y	 b.x, b.y
	x0 y0			x1 y0
	*/
	// back bottom left corner: min(x), min(y), min(z)
	// front upper right corner: max(x), max(y), max(z)

	box = aabb(vec3(fmin(fmin(a.x(), b.x()), c.x()), fmin(fmin(a.y(), b.y()), c.y()), fmin(fmin(a.z(), b.z()), c.z() - 0.01)),
		vec3(fmax(fmax(a.x(), b.x()), c.x()), fmax(fmax(a.y(), b.y()), c.y()), fmax(fmax(a.z(), b.z()), c.z()) + 0.01));
	return true;
}
#endif