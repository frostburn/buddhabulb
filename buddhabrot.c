#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include "buddhabrot.h"

double unifrm (){
    return 9.313225750491594e-10 * rand() - 1;
}

void uniform(double *x, double *y) {
    int r = rand();
    *x = 3.0518043793392844e-05 * (r & 65535) - 1;
    *y = 6.103701895199438e-05 * (r >> 16) - 1;

}

void gauss(double *x, double *y) {
    double tx, ty;
    uniform(x, y);
    uniform(&tx, &ty);
    *x += tx;
    *y += ty;
    uniform(&tx, &ty);
    *x += tx;
    *y += ty;
    uniform(&tx, &ty);
    *x += tx;
    *y += ty;
}

size_t to_index(double x, double y, double cx, double cy, double zoom, size_t width, size_t height) {
    // Aspect ratio 1:1 with a little zoom.
    // x = (x + 2.2) * 0.333;
    // y = (y + 1.5) * 0.333;

    x -= cx;
    y -= cy;
    x *= zoom;
    y *= zoom;

    x += 0.5;
    y += 0.5;
    x *= ((double) height) / ((double) width);

    if (x < 0 || x > 1 || y < 0 || y > 1) {
        return -1;
    }
    size_t ix = (size_t) (x * width);
    size_t iy = (size_t) (y * height);
    size_t index = ix + width * iy;
    if (index >= width * height) {
        return -1;
    }
    return index;
}

int main(int argc, char *argv[]) {
    assert(RAND_MAX == 2147483647);

    if (argc < 7) {
        printf("Need width, height, number of layers, center x, center y and zoom.\n");
        return 1;
    }
    int width = atoi(argv[1]);
    int height = atoi(argv[2]);
    int num_layers = atoi(argv[3]);
    double center_x = atof(argv[4]);
    double center_y = atof(argv[5]);
    double zoom = atof(argv[6]);
    size_t num_pixels = width * height;
    size_t max_iter = 2000;
    size_t save_interval = 100000000;
    double bailout = 16;
    #ifdef FULL_DECOMPOSE
        for (int i = 0; i < max_iter - 1; i++) {
            for (int j = 0; j < i; j++) {
                num_layers++;
            }
        }
    #endif
    size_t num_bins = num_pixels * num_layers;
    bin_t *bins = calloc(num_bins, sizeof(bin_t));

    FILE *f;
    if (argc == 8) {
        f = fopen("buddhabrot_bins.dat", "rb");
        assert(fread((void*) bins, sizeof(bin_t), num_bins, f));
        fclose(f);
        printf("Load done\n");
    }

    int *trajectory = malloc(max_iter * sizeof(int));
    size_t i = 0;
    while (1) {
        if (i % save_interval == save_interval - 1) {
            f = fopen("buddhabrot_bins.dat", "wb");
            fwrite((void*) bins, sizeof(bin_t), num_bins, f);
            fclose(f);
            printf("Save done\n");
        }
        i++;

        double cx, cy;
        gauss(&cx, &cy);
        cx *= 0.1;
        cy *= 0.1;
        cx -= 1.3;
        cy += 0.2;

        // Check bailout.
        double cx2 = cx * cx;
        double cy2 = cy * cy;
        if (cx2 + cy2 > bailout) {
            continue;
        }

        // Check main cardioid and 2-period bulb.
        double q = cx - 0.25;
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
                #ifdef FULL_DECOMPOSE
                    size_t layer = ((j - 1) * (j - 2)) / 2;
                #endif
                for (size_t k = 0; k < j - 1; k += 1) {
                    size_t index = trajectory[k];
                    if (index >= 0) {
                        #ifdef FULL_DECOMPOSE
                            index += (layer + k) * num_pixels;
                        #else
                            index += k * num_pixels;
                        #endif
                        if (index < num_bins && bins[index] < BIN_MAX) {
                            bins[index]++;
                        }
                    }
                }
                break;
            }
            y = 2 * x*y + cy;
            x = x2 - y2 + cx;
            trajectory[j] = to_index(x, y, center_x, center_y, zoom, width, height);
        }
    }

    return 0;
}
