#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include "buddhabrot.h"

typedef unsigned char color_t;

void get_layer(int iteration, FILE *f, bin_t *bins, int num_pixels) {
    fseek(f, iteration * num_pixels * sizeof(bin_t), SEEK_SET);
    assert(fread((void*) bins, sizeof(bin_t), num_pixels, f));
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("Need width, height and layer.\n");
        return 1;
    }
    int width = atoi(argv[1]);
    int height = atoi(argv[2]);
    int layer = atoi(argv[3]);
    int num_pixels = width * height;

    bin_t *bins = calloc(3 * num_pixels, sizeof(bin_t));
    bin_t *temp = malloc(num_pixels * sizeof(bin_t));

    FILE *f = fopen("buddhabulb_bins.dat", "rb");
    for (int i = layer; i < 300; i += 2) {
        get_layer(i, f, temp, num_pixels);
        for (int j = 0; j < num_pixels; j++) {
            int c = (i / 6) % 3;
            if (c == 0) {
                bins[3 * j + c] += temp[j];
            }
            if (c == 1) {
                bins[3 * j + c] += temp[j] * exp(-0.001 * pow(i - 180, 2)) * 13;
            }
            if (c == 2) {
                bins[3 * j + c] += temp[j] * exp(-0.000009 * pow(i - 1020, 2)) * 800;
            }
        }
    }

    fclose(f);

    bin_t bin_max = 0;
    for (int i = 0; i < 3 * num_pixels; i++) {
        if (bins[i] > bin_max) {
            bin_max = bins[i];
        }
    }

    printf("%d\n", bin_max);

    color_t *img = malloc(3 * num_pixels * sizeof(color_t));
    for (int i = 0; i < 3 * num_pixels; i++) {
        img[i] = (color_t) (255 * tanh(5.0 * ((double) bins[i]) / bin_max));
    }
    f = fopen("out.raw", "wb");
    fwrite((void*) img, sizeof(color_t), 3 * num_pixels, f);
    fclose(f);

    return 0;
}
