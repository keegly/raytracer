#include <fstream>
#include <random>
#include <float.h>
#include "sphere.h"
#include "hitable_list.h"
#include "camera.h"
#include "material.h"
#include "bvh.h"

std::default_random_engine generator;
//std::uniform_real_distribution<float> distribution(-1.0, 1.0);
std::uniform_real_distribution<float> drand(0, 1.0);

vec3 color(const ray& r, hitable *world, int depth) {
	hit_record rec;
	if (world->hit(r, 0.001, FLT_MAX, rec)) {
		ray scattered;
		vec3 attenuation;
		if (depth < 50 && rec.mat_ptr->scatter(r, rec, attenuation, scattered)) {
			return attenuation*color(scattered, world, depth + 1);
		}
		else {
			return vec3(0, 0, 0);
		}
	}
	else {
		vec3 unit_direction = unit_vector(r.direction());
		float t = 0.5 * (unit_direction.y() + 1.0);
		return (1.0 - t) * vec3(1.0, 1.0, 1.0) + t * vec3(0.5, 0.7, 1.0);
	}
}

hitable *random_scene() {
	int n = 50000;
	hitable **list = new hitable*[n + 1];
	texture *checker = new checker_texture(new constant_texture(vec3(0.2, 0.3, 0.1)), new constant_texture(vec3(0.9, 0.9, 0.9)));
	list[0] = new sphere(vec3(0, -1000, 0), 1000, new lambertian(checker));
	int i = 1;
	for (int a = -10; a < 10; a++) {
		for (int b = -10; b < 10; b++) {
			float choose_mat = drand(generator);
			vec3 center(a + 0.9 + drand(generator), 0.2, b + 0.9*drand(generator));
			if ((center - vec3(4, 0.2, 0)).length() > 0.9) {
				if (choose_mat < 0.8) { // diffuse
					list[i++] = new moving_sphere(center, center + vec3(0, 0.5*drand(generator), 0),
						0.0, 1.0, 0.2, new lambertian(new constant_texture(
							vec3(drand(generator)*drand(generator),
								drand(generator)*drand(generator),
								drand(generator)*drand(generator)))));
				}
				else if (choose_mat < 0.95) { // metal
					list[i++] = new sphere(center, 0.2, new metal(
						vec3(0.5*(1 + drand(generator)),
							0.5*(1 + drand(generator)),
							0.5*(1 + drand(generator))),
						0.5*drand(generator)));
				}
				else { // glass
					list[i++] = new sphere(center, 0.2, new dielectric(1.5));
				}
			}
		}
	}

	list[i++] = new sphere(vec3(0, 1, 0), 1.0, new dielectric(1.5));
	list[i++] = new sphere(vec3(-4, 1, 0), 1.0, new lambertian(new constant_texture(vec3(0.4, 0.2, 0.1))));
	list[i++] = new sphere(vec3(4, 1, 0), 1.0, new metal(vec3(0.7, 0.6, 0.5), 0.0));

	//return new hitable_list(list, i);
	return new bvh_node(list, i, 0.0, 1.0);
}

hitable *two_spheres() {
	texture *checker = new checker_texture(new constant_texture(vec3(0.2, 0.3, 0.1)), new constant_texture(vec3(0.9, 0.9, 0.9)));
	int n = 50;
	hitable **list = new hitable*[n + 1];
	list[0] = new sphere(vec3(0, -10, 0), 10, new lambertian(checker));
	list[1] = new sphere(vec3(0, 10, 0), 10, new lambertian(checker));

	return new bvh_node(list, 2, 0.0, 1.0);
}

int main() {
	// Seed RNG
	generator.seed(std::random_device()());
	int nx = 400;
	int ny = 200;
	int ns = 50;
	std::ofstream ost{ "scene.ppm" };
	ost << "P3\n" << nx << " " << ny << "\n255\n";

	constexpr double M_PI = 3.14159265358979323846;

	const int NUM_SPHERES = 5;
	hitable *list[NUM_SPHERES];
	list[0] = new sphere(vec3(0, 0, -1), 0.5, new lambertian(new constant_texture(vec3(0.1, 0.2, 0.5)))); // Blue middle
	list[1] = new sphere(vec3(0, -100.5, -1), 100, new lambertian(new constant_texture(vec3(0.8, 0.8, 0.0)))); // Giant green that acts as ground
	list[2] = new sphere(vec3(1, 0, -1), 0.5, new metal(vec3(0.8, 0.6, 0.2))); // Metal right
	list[3] = new sphere(vec3(-1, 0, -1), 0.5, new dielectric(1.5)); // These two act as a sort of glass bubble
	list[4] = new sphere(vec3(-1, 0, -1), -0.45, new dielectric(1.5)); // Only work together though?

	//hitable *world = new hitable_list(list, NUM_SPHERES);
	hitable *world = new bvh_node(list, NUM_SPHERES, 0.0, 1.0);
	world = random_scene();
	vec3 lookfrom(13, 2, 3);
	vec3 lookat(0, 0, 0);
	//float dist_to_focus = (lookfrom - lookat).length();
	float dist_to_focus = 10.0;
	float aperture = 0.0;
	camera cam(lookfrom, lookat, vec3(0, 1, 0), 20, float(nx) / float(ny), aperture, dist_to_focus, 0.0, 1.0);

	for (int j = ny - 1; j >= 0; j--) {
		for (int i = 0; i < nx; i++) {
			vec3 col(0, 0, 0);
			for (int s = 0; s < ns; s++) {
				float u = float(i + drand(generator)) / float(nx);
				float v = float(j + drand(generator)) / float(ny);
				ray r = cam.get_ray(u, v);
				vec3 p = r.point_at_parameter(2.0);
				col += color(r, world, 0);
			}

			col /= float(ns);
			col = vec3(sqrt(col[0]), sqrt(col[1]), sqrt(col[2]));
			int ir = int(255.99*col[0]);
			int ig = int(255.99*col[1]);
			int ib = int(255.99*col[2]);

			ost << ir << " " << ig << " " << ib << "\n";
		}
	}
}