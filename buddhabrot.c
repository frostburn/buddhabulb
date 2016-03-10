#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef unsigned char color_t;

double uniform() {
    return 2 * (((double) rand()) / RAND_MAX) - 1;
}

int to_index(double x, double y, int width, int height) {
    x = (x + 2.5) * 0.25;
    y = (y + 2) * 0.25;
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
    v *= 3;
    if (v > 255) {
        return 255;
    }
    return (color_t) v;
}

int main() {
    printf("moi\n");
    int width = 600;
    int height = 600;
    int num_pixels = width * height;
    double *rbins = calloc(num_pixels, sizeof(double));
    double *gbins = calloc(num_pixels, sizeof(double));
    double *bbins = calloc(num_pixels, sizeof(double));

    double bailout = 4.0;
    int max_iter = 1000;
    int samples = 1000000000;
    int *trajectory = malloc(max_iter * sizeof(int));
    for (int i = 0; i < samples; i++) {
        double x = 4 * uniform();
        double y = 4 * uniform();
        double zx = 0.0;
        double zy = 0.0;
        for (int j = 0; j < max_iter; j++) {
            double t = zx;
            zx = zx*zx - zy*zy + x;
            zy = 2 * t*zy + y;
            trajectory[j] = to_index(zx, zy, width, height);
            if (zx*zx + zy*zy > bailout) {
                for (int k = 1; k < j + 1; k++) {
                    if (k % 3 != 0 || k % 5 != 0) {
                        continue;
                    }
                    int index = trajectory[k];
                    if (index >= 0) {
                        rbins[index] += exp(-0.0001*(k-15)*(k-15)) * 0.5;
                        gbins[index] += exp(-0.0001*(k-300)*(k-300));
                        bbins[index] += exp(-0.0001*(k-600)*(k-600)) * 5;
                    }
                }
                break;
            }
        }
    }

    FILE *f = fopen("temp.raw", "wb");
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
