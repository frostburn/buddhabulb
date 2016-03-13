#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <omp.h>

typedef unsigned char color_t;
typedef double bin_t;

double uniform() {
    return 2 * (((double) rand()) / RAND_MAX) - 1;
}

int to_index(double x, double y, double z, int width, int height) {
    x += 0.15 * z;
    y += 0.15 * z;
    x = (x + 1.5) * 0.3333;
    y = (y + 1.5) * 0.3333;
    if (x < 0 || x > 1 || y < 0 || y > 1) {
        return -1;
    }
    int ix = (int) (x * width);
    int iy = (int) (y * height);
    int index = ix + width * iy;
    if (index >= width * height) {
        return -1;
    }
    return index;
}

color_t clamp(double v) {
    v *= 1;
    if (v > 255) {
        return 255;
    }
    return (color_t) v;
}

int main() {
    int width = 100;
    int height = 100;
    int num_pixels = width * height;
    bin_t *rbins = calloc(num_pixels, sizeof(bin_t));
    bin_t *gbins = calloc(num_pixels, sizeof(bin_t));
    bin_t *bbins = calloc(num_pixels, sizeof(bin_t));

    double bailout = 4.0;
    size_t max_iter = 2000;
    size_t samples = 100000000ULL;
    int **trajectories = malloc(omp_get_max_threads() * sizeof(int*));
    for (int i = 0; i < omp_get_max_threads(); i++) {
        trajectories[i] = malloc(max_iter * sizeof(int));
    }
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        srand(time(0) ^ tid);
        printf("%d: %d\n", tid, rand());
        #pragma omp for
        for (size_t i = 0; i < samples; i++) {
            double cx = 4 * uniform();
            double cy = 4 * uniform();
            double cz = 4 * uniform();
            if (i % 1000000 == 0) {
                printf("%zu\n", (i * 100) / samples);
            }
            double x = 0.0;
            double y = 0.0;
            double z = 0.0;
            for (size_t j = 0; j < max_iter; j++) {
                double rxy = x*x + y*y;
                double r = rxy + z*z;
                if (r > bailout) {
                    for (size_t k = 3; k < j; k++) {
                        if (k % 3 != 0) {
                            continue;
                        }
                        int index = trajectories[tid][k];
                        if (index >= 0) {
                            #pragma omp critical
                            {
                                rbins[index] += exp(-0.0001*(k-15)*(k-15)) * 0.5;
                                gbins[index] += exp(-0.0001*(k-300)*(k-300));
                                bbins[index] += exp(-0.0001*(k-600)*(k-600)) * 4;
                            }
                        }
                    }
                    break;
                }
                r *= r * r;
                double phi = atan2(y, x);
                double theta = atan2(sqrt(rxy), z);
                x = r * sin(6 * theta);
                y = x * sin(6 * phi) + cy;
                x = x * cos(6 * phi) + cx;
                z = r * cos(6 * theta) + cz;
                trajectories[tid][j] = to_index(x, y, z, width, height);
            }
        }
    }

    FILE *f = fopen("bins.dat", "wb");
    fwrite((void*) rbins, sizeof(bin_t), num_pixels, f);
    fwrite((void*) gbins, sizeof(bin_t), num_pixels, f);
    fwrite((void*) bbins, sizeof(bin_t), num_pixels, f);
    fclose(f);

    f = fopen("temp.raw", "wb");
    for (int i = 0; i < num_pixels; i++) {
        color_t red = clamp(rbins[i]);
        fwrite((void*) &red, sizeof(color_t), 1, f);
        color_t green = clamp(gbins[i]);
        fwrite((void*) &green, sizeof(color_t), 1, f);
        color_t blue = clamp(bbins[i]);
        fwrite((void*) &blue, sizeof(color_t), 1, f);
    }
    fclose(f);
    return 0;
}
