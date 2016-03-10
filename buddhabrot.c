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
    v *= 0.5;
    if (v > 255) {
        return 255;
    }
    return (color_t) v;
}

int main() {
    int width = 100;
    int height = 100;
    int num_pixels = width * height;
    double *rbins = calloc(num_pixels, sizeof(double));
    double *gbins = calloc(num_pixels, sizeof(double));
    double *bbins = calloc(num_pixels, sizeof(double));

    double bailout = 4.0;
    int max_iter = 1000;
    int samples = 30000000;
    int *trajectory = malloc(max_iter * sizeof(int));
    for (int i = 0; i < samples; i++) {
        double cx = 2 * uniform();
        double cy = 2 * uniform();

        // Check main cardioid and 2-period bulb.
        double q = cx - 0.25;
        double cy2 = cy * cy;
        q = q*q + cy2;
        if (q * (q + (cx - 0.25)) < 0.25 * cy2) {
            continue;
        }
        if ((cx + 1) * (cx + 1) + cy2 < 0.0625) {
            continue;
        }

        double x = 0.0;
        double y = 0.0;
        for (int j = 0; j < max_iter; j++) {
            double x2 = x*x;
            double y2 = y*y;
            if (x2 + y2 > bailout) {
                for (int k = 1; k < j; k++) {
                    if (k % 3 != 0 || k % 5 != 0) {
                        continue;
                    }
                    int index = trajectory[k];
                    if (index >= 0) {
                        rbins[index] += exp(-0.00001*(k-20)*(k-20)) * 0.6;
                        gbins[index] += exp(-0.00006*(k-220)*(k-220));
                        bbins[index] += exp(-0.0001*(k-500)*(k-500)) * 5;
                    }
                }
                break;
            }
            y = 2 * x*y + cy;
            x = x2 - y2 + cx;
            trajectory[j] = to_index(x, y, width, height);
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
