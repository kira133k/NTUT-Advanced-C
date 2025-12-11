#include <cstdio>
#include <vector>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <fstream>
#include "BVH.h"
#include "Doraemon.h"
#include "Pikachu.h"
#include "Vector3.h"

using std::vector;

using namespace std;

// Save results
struct ExperimentResult
{
	int scene;
	int objectCount;
	double buildTime;
	double renderTime;
};

// Return a random number in [0,1]
float rand01()
{
	return rand() * (1.f / RAND_MAX);
}

// Return a random vector with each component in the range [-1,1]
Vector3 randVector3()
{
	return Vector3(rand01(), rand01(), rand01()) * 2.f - Vector3(1, 1, 1);
}

void Experiment(int N, int sceneScale, vector<ExperimentResult> &results)
{
	int width = 1024;
	int height = 1024;

	printf(">>> Running Experiment: Resolution %dx%d (Objects: %d) <<<\n", width, height, N);
	vector<Object *> objects;

	// Mix object
	for (size_t i = 0; i < N; ++i)
	{
		if (i % 2 == 0)
		{
			objects.push_back(new Doraemon(randVector3() * (float)sceneScale, 0.01f));
		}
		else
		{
			objects.push_back(new Pikachu(randVector3() * (float)sceneScale, 0.01f));
		}
	}

	// BVH build time
	auto start_BVH = std::chrono::high_resolution_clock::now();

	BVH bvh(&objects);

	auto end_BVH = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsed_build = end_BVH - start_BVH;
	printf("   [Time] BVH Construction: %.5f seconds\n", elapsed_build.count());

	// Allocate space for some image pixels
	float *pixels = new float[width * height * 3];

	// Create a camera from position and focus point
	Vector3 camera_position(1.6, 1.3, 1.6);
	Vector3 camera_focus(0, 0, 0);
	Vector3 camera_up(0, 1, 0);

	// Camera tangent space
	Vector3 camera_dir = normalize(camera_focus - camera_position);
	Vector3 camera_u = normalize(camera_dir ^ camera_up);
	Vector3 camera_v = normalize(camera_u ^ camera_dir);

	// Rendering time
	printf("   [Rendering] %dx%d image...\n", width, height);
	auto start_render = std::chrono::high_resolution_clock::now();

	// Raytrace over every pixel
	for (size_t i = 0; i < width; ++i)
	{
		for (size_t j = 0; j < height; ++j)
		{
			size_t index = 3 * (width * j + i);

			float u = (i + .5f) / (float)(width - 1) - .5f;
			float v = (height - 1 - j + .5f) / (float)(height - 1) - .5f;
			float fov = .5f / tanf(70.f * 3.14159265 * .5f / 180.f);

			// This is only valid for square aspect ratio images
			Ray ray(camera_position, normalize(u * camera_u + v * camera_v + fov * camera_dir));

			IntersectionInfo I;
			bool hit = bvh.getIntersection(ray, &I, false);

			if (!hit)
			{
				pixels[index] = pixels[index + 1] = pixels[index + 2] = 0.f;
			}
			else
			{
				// Just for fun, we'll make the color based on the normal
				const Vector3 normal = I.object->getNormal(I);
				const Vector3 color(fabs(normal.x), fabs(normal.y), fabs(normal.z));

				pixels[index] = color.x;
				pixels[index + 1] = color.y;
				pixels[index + 2] = color.z;
			}
		}
	}

	auto end_render = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsed_render = end_render - start_render;
	printf("   [Time] Rendering: %.5f seconds\n", elapsed_render.count());

	// Save results
	results.push_back({sceneScale, N, elapsed_build.count(), elapsed_render.count()});

	char filename[64];
	sprintf(filename, "render_Scale=%d_N=%d.ppm", sceneScale, N);

	FILE *image = fopen(filename, "wb");
	fprintf(image, "P6\n%d %d\n255\n", width, height);
	for (size_t j = 0; j < height; ++j)
	{
		for (size_t i = 0; i < width; ++i)
		{
			size_t index = 3 * (width * j + i);
			unsigned char r = std::max(std::min(pixels[index] * 255.f, 255.f), 0.f);
			unsigned char g = std::max(std::min(pixels[index + 1] * 255.f, 255.f), 0.f);
			unsigned char b = std::max(std::min(pixels[index + 2] * 255.f, 255.f), 0.f);
			fprintf(image, "%c%c%c", r, g, b);
		}
	}
	fclose(image);
	printf("   [Output] Saved to %s\n", filename);

	// Cleanup
	delete[] pixels;
	for (Object *obj : objects)
		delete obj;
	objects.clear();

	printf("------------------------------------------------\n");
}

int main(int argc, char **argv)
{
	srand(12345);

	vector<ExperimentResult> results;

	vector<int> test_cases = {100, 500, 1000, 2000};
	vector<float> scales = {1, 2, 4, 6};

	printf("------------------------------------------------\n");

	for (int n : test_cases)
	{
		for (float s : scales)
		{
			Experiment(n, s, results);
		}
	}

	ofstream outFile("report.txt");

	if (outFile.is_open())
	{
		outFile << "=============================================================================" << endl;
		outFile << "                             Performance  Report                             " << endl;
		outFile << "=============================================================================" << endl;
		outFile << "| " << left << setw(12) << "Scene Scale"
				<< " | " << setw(10) << "Objects(N)"
				<< " | " << setw(20) << "Build Time (s)"
				<< " | " << setw(20) << "Render Time (s)" << " |" << endl;
		outFile << "|--------------|------------|----------------------|----------------------|" << endl;

		for (const auto &res : results)
		{
			char scaleStr[20];
			sprintf(scaleStr, "%d", res.scene);
			outFile << "| " << left << setw(12) << scaleStr
					<< " | " << setw(10) << res.objectCount
					<< " | " << setw(20) << fixed << setprecision(5) << res.buildTime
					<< " | " << setw(20) << fixed << setprecision(5) << res.renderTime << " |" << endl;
		}
		outFile << "=============================================================================" << endl;

		printf("\n[Success] Report saved to \"report.txt\"\n");
		outFile.close();
	}
	else
	{
		printf("\n[Error] Unable to save report to file.\n");
	}

	printf("\n");
	printf("=========================================================================\n");
	printf("                       Performance  Report (Console)                     \n");
	printf("=========================================================================\n");
	printf("| %-12s | %-10s | %-20s | %-20s |\n", "Scene Scale", "Objects(N)", "Build Time (s)", "Render Time (s)");
	printf("|--------------|------------|----------------------|----------------------|\n");

	for (const auto &res : results)
	{
		cout << "| " << setw(12) << fixed << setprecision(1) << res.scene
			 << " | " << setw(10) << res.objectCount
			 << " | " << setw(20) << fixed << setprecision(5) << res.buildTime
			 << " | " << setw(20) << fixed << setprecision(5) << res.renderTime << " |" << endl;
	}
	printf("=========================================================================\n");

	printf("All tests finished.\n");

	return 0;
}