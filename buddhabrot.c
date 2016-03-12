#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include "buddhabrot.h"

double uniform() {
    return 2 * (((double) rand()) / RAND_MAX) - 1;
}

size_t to_index(double x, double y, size_t width, size_t height) {
    // Aspect ratio 1:1 with a little zoom.
    // x = (x + 2.2) * 0.333;
    // y = (y + 1.5) * 0.333;

    // Aspect ratio 3:2
    x = (x + 3) * 0.1666666666666;
    y = (y + 2) * 0.25;

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
    size_t width = 1920;
    size_t height = 1280;
    size_t num_pixels = width * height;
    size_t max_iter = 2000;
    size_t num_layers = 0;
    size_t max_layer = 0;
    size_t save_interval = 1000000000;
    #ifdef FULL_DECOMPOSE
        for (int i = 0; i < max_iter - 1; i++) {
            for (int j = 0; j < i; j++) {
                num_layers++;
            }
        }
    #else
        num_layers = 150;
    #endif
    size_t num_bins = num_pixels * num_layers;
    bin_t *bins = calloc(num_bins, sizeof(bin_t));

    FILE *f;
    if (argc > 1) {
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

        double cx = 3 * uniform();
        double cy = 3 * uniform();

        // Check bailout.
        double cx2 = cx * cx;
        double cy2 = cy * cy;
        if (cx2 + cy2 > 9) {
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
            if (x2 + y2 > 9) {
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
            trajectory[j] = to_index(x, y, width, height);
        }
    }

    return 0;
}
