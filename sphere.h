#ifndef SPHEREH
#define SPHEREH

#include "hitable.h"

class material;

class sphere : public hitable {
public:
	sphere() {}
	sphere(vec3 cen, float r, material *m) : center(cen), radius(r), mat(m) {};
	virtual bool hit(const ray& r, float tmin, float tmax, hit_record& rec) const;
	vec3 center;
	float radius;
	material *mat;
};

bool sphere::hit(const ray& r, float t_min, float t_max, hit_record& rec) const {
	rec.mat_ptr = mat; //update hit record material pointer to the proper material this sphere was given when it was created
	vec3 oc = r.origin() - center;
	float a = dot(r.direction(), r.direction());
	float b = dot(oc, r.direction());
	float c = dot(oc, oc) - radius*radius;
	float discriminant = b*b - a*c;
	if (discriminant > 0) {
		float temp = (-b - sqrt(b*b - a*c)) / a;
		if (temp < t_max && temp > t_min) {
			rec.t = temp;
			rec.p = r.point_at_parameter(rec.t);
			rec.normal = (rec.p - center) / radius;
			return true;
		}
		temp = (-b + sqrt(b*b - a*c)) / a;
		if (temp < t_max && temp > t_min) {
			rec.t = temp;
			rec.p = r.point_at_parameter(rec.t);
			rec.normal = (rec.p - center) / radius;
			return true;
		}
	}
	return false;
}

#endif