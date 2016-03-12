#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include "buddhabrot.h"

typedef unsigned char color_t;

#ifdef FULL_DECOMPOSE
    void get_layer(int escape, int iteration, FILE *f, bin_t *bins, int num_pixels) {
        assert(escape > iteration);
        int layer = (escape * (escape - 1)) / 2;
        fseek(f, (layer + iteration) * num_pixels * sizeof(bin_t), SEEK_SET);
        assert(fread((void*) bins, sizeof(bin_t), num_pixels, f));
    }
#else
    void get_layer(int iteration, FILE *f, bin_t *bins, int num_pixels) {
        fseek(f, iteration * num_pixels * sizeof(bin_t), SEEK_SET);
        assert(fread((void*) bins, sizeof(bin_t), num_pixels, f));
    }
#endif

int main(int argc, char *argv[]) {
    int width = 1920;
    int height = 1280;
    int num_pixels = width * height;
    int max_iter = 100;
    #ifdef FULL_DECOMPOSE
        for (int i = 0; i < max_iter - 1; i++) {
            for (int j = 0; j < i; j++) {
                num_layers++;
            }
        }
    #else
        int num_layers = 150;
    #endif
    bin_t *bins = calloc(num_pixels, sizeof(bin_t));
    bin_t *temp = malloc(num_pixels * sizeof(bin_t));

    FILE *f = fopen("buddhabrot_bins.dat", "rb");
    #ifdef FULL_DECOMPOSE
        for (int i = 7; i < max_iter - 1; i++) {
            get_layer(i, 6, f, temp, num_pixels);
            for (int j = 0; j < num_pixels; j++) {
                bins[j] += temp[j];
            }
        }
    #else
        get_layer(149, f, bins, num_pixels);
    #endif
    fclose(f);

    bin_t bin_max = 0;
    for (int i = 0; i < num_pixels - 1; i++) {
        if (bins[i] > bin_max) {
            bin_max = bins[i];
        }
    }

    printf("%d\n", bin_max);

    color_t *img = malloc(num_pixels * sizeof(color_t));
    for (int i = 0; i < num_pixels; i++) {
        img[i] = (color_t) (255 * bins[i] / bin_max);
    }
    f = fopen("buddhabrot.raw", "wb");
    fwrite((void*) img, sizeof(color_t), num_pixels, f);
    fclose(f);

    return 0;
}
