#include <fstream>
#include <random>
#include <float.h>
#include "sphere.h"
#include "hitable_list.h"
#include "camera.h"
#include "material.h"
#include "bvh.h"

//needed for rng
#include "perlin.h"

// needed for image textures
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

const float _pi = 3.14159265358979f;

static float get_rand() {
	static std::default_random_engine generator;
	static std::uniform_real_distribution<float> drand(0, 1.0);
	// Seed RNG
	generator.seed(std::random_device()());

	float num = drand(generator);
	return num;
}

vec3 color(const ray& r, hitable *world, int depth) {
	hit_record rec;
	if (world->hit(r, 0.001f, FLT_MAX, rec)) {
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
		float t = 0.5f * (unit_direction.y() + 1.0f);
		return (1.0f - t) * vec3(1.0f, 1.0f, 1.0f) + t * vec3(0.5f, 0.7f, 1.0f);
	}
}

hitable *random_scene() {
	int n = 50000;
	hitable **list = new hitable*[n + 1];
	texture *checker = new checker_texture(new constant_texture(vec3(0.2f, 0.3f, 0.1f)), new constant_texture(vec3(0.9f, 0.9f, 0.9f)));
	list[0] = new sphere(vec3(0, -1000, 0), 1000, new lambertian(checker));
	int i = 1;
	for (int a = -10; a < 10; a++) {
		for (int b = -10; b < 10; b++) {
			float choose_mat = get_rand();
			vec3 center(a + 0.9f + get_rand(), 0.2f, b + 0.9f*get_rand());
			if ((center - vec3(4, 0.2f, 0)).length() > 0.9f) {
				if (choose_mat < 0.8f) { // diffuse
					list[i++] = new moving_sphere(center, center + vec3(0, 0.5f*get_rand(), 0),
						0.0f, 1.0f, 0.2f, new lambertian(new constant_texture(
							vec3(get_rand()*get_rand(),
								get_rand()*get_rand(),
								get_rand()*get_rand()))));
				}
				else if (choose_mat < 0.95f) { // metal
					list[i++] = new sphere(center, 0.2f, new metal(
						vec3(0.5f*(1 + get_rand()),
							0.5f*(1 + get_rand()),
							0.5f*(1 + get_rand())),
						0.5f*get_rand()));
				}
				else { // glass
					list[i++] = new sphere(center, 0.2f, new dielectric(1.5f));
				}
			}
		}
	}

	list[i++] = new sphere(vec3(0, 1, 0), 1.0, new dielectric(1.5f));
	list[i++] = new sphere(vec3(-4, 1, 0), 1.0, new lambertian(new constant_texture(vec3(0.4f, 0.2f, 0.1f))));
	list[i++] = new sphere(vec3(4, 1, 0), 1.0, new metal(vec3(0.7f, 0.6f, 0.5f), 0.0f));

	//return new hitable_list(list, i);
	return new bvh_node(list, i, 0.0, 1.0);
}

hitable *two_spheres() {
	texture *checker = new checker_texture(new constant_texture(vec3(0.2f, 0.3f, 0.1f)), new constant_texture(vec3(0.9f, 0.9f, 0.9f)));
	hitable **list = new hitable*[2];
	list[0] = new sphere(vec3(0, -10, 0), 10, new lambertian(checker));
	list[1] = new sphere(vec3(0, 10, 0), 10, new lambertian(checker));

	return new bvh_node(list, 2, 0.0, 1.0);
}

hitable *two_perlin_spheres() {
	texture *pertext = new noise_texture(4);
	hitable **list = new hitable*[2];
	list[0] = new sphere(vec3(0, -1000, 0), 1000, new lambertian(pertext));
	list[1] = new sphere(vec3(0, 2, 0), 2, new lambertian(pertext));
	return new bvh_node(list, 2, 0.0, 1.0);
}

hitable *earth() {
	int nx, ny, nn;
	unsigned char *tex_data = stbi_load("earthmap.jpg", &nx, &ny, &nn, 0);
	material *mat = new lambertian(new image_texture(tex_data, nx, ny));

	return new sphere(vec3(0, 0, 0), 2, mat);
}

int main() {
	int nx = 400;
	int ny = 200;
	int ns = 50;
	std::ofstream ost{ "scene.ppm" };
	ost << "P3\n" << nx << " " << ny << "\n255\n";

	const int NUM_SPHERES = 5;
	hitable *list[NUM_SPHERES];
	list[0] = new sphere(vec3(0, 0, -1), 0.5f, new lambertian(new constant_texture(vec3(0.1f, 0.2f, 0.5f)))); // Blue middle
	list[1] = new sphere(vec3(0, -100.5f, -1), 100, new lambertian(new constant_texture(vec3(0.8f, 0.8f, 0.0f)))); // Giant green that acts as ground
	list[2] = new sphere(vec3(1, 0, -1), 0.5f, new metal(vec3(0.8f, 0.6f, 0.2f))); // Metal right
	list[3] = new sphere(vec3(-1, 0, -1), 0.5f, new dielectric(1.5f)); // These two act as a sort of glass bubble
	list[4] = new sphere(vec3(-1, 0, -1), -0.45f, new dielectric(1.5f)); // Only work together though?

	//hitable *world = new hitable_list(list, NUM_SPHERES);
	/*hitable *world = new bvh_node(list, NUM_SPHERES, 0.0, 1.0);
	world = random_scene();
	world = two_spheres();
	world = two_perlin_spheres();*/
	hitable *world = earth();
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
				float u = float(i + get_rand()) / float(nx);
				float v = float(j + get_rand()) / float(ny);
				ray r = cam.get_ray(u, v);
				//vec3 p = r.point_at_parameter(2.0);
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