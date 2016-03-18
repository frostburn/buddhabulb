#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include "buddhabrot.h"

// #define SINGLE_LAYER
#define SINGLE_CHANNEL

// At the time of this commit this was pretty:
// ./buddhabrot 1920 1280 150 -0.89 0 0.65
// ./buddhabrot_process 1920 1280 21 150 0

typedef unsigned char color_t;

void get_layer(size_t iteration, FILE *f, bin_t *bins, int num_pixels) {
    fseek(f, iteration * num_pixels * sizeof(bin_t), SEEK_SET);
    assert(fread((void*) bins, sizeof(bin_t), num_pixels, f));
}

int main(int argc, char *argv[]) {
    if (argc < 6) {
        printf("Need width, height, layer, number of layers and normalizer.\n");
        return 1;
    }
    int width = atoi(argv[1]);
    int height = atoi(argv[2]);
    int layer = atoi(argv[3]);
    int num_layers = atoi(argv[4]);
    bin_t normalizer = atoi(argv[5]);
    int num_pixels = width * height;

    double *bins = calloc(3 * num_pixels, sizeof(double));
    bin_t *temp = malloc(num_pixels * sizeof(bin_t));

    FILE *f = fopen("buddhabrot_bins.dat", "rb");

    #ifdef SINGLE_LAYER
        get_layer(layer, f, temp, num_pixels);
        for (int j = 0; j < num_pixels - 1; j++) {
            for (int c = 0; c < 3; c++) {
                bins[3 * j + c] += temp[j];
            }
        }
    #else
        for (int i = layer; i < num_layers; i++) {
            get_layer(i, f, temp, num_pixels);
            printf("%d\n", i);
            for (int c = 0; c < 3; c++) {
                double mul = 1 + (0.5 * c + 0.1) * sqrt(i) * 0.11;
                if (c == 2) {
                    mul *= exp(-pow(0.001 * (i - 140), 2));
                }
                for (int j = 0; j < num_pixels - 1; j++) {
                        bins[3 * j + c] += temp[j] * mul;
                }
            }
        }
    #endif

    fclose(f);

    bin_t bin_max = 0;
    for (int i = 0; i < 3 * num_pixels; i++) {
        if (bins[i] > bin_max) {
            bin_max = bins[i];
        }
    }

    printf("%d\n", bin_max);

    if (!normalizer) {
        normalizer = bin_max;
    }

    color_t *img = malloc(3 * num_pixels * sizeof(color_t));
    for (int i = 0; i < 3 * num_pixels; i++) {
        double v = ((double) bins[i]) / normalizer;
        v *= 5;
        #ifdef SINGLE_CHANNEL
            if (i % 3 == 2) {
                v = tanh(6 * (v - 0.66)) * 0.5 + 0.5;
            }
            else if (i % 3 == 1) {
                v = tanh(4 * (v - 0.33)) * 0.5 + 0.5;
            }
            else {
                v = tanh(4 * v);
            }
        #else
            v = tanh(5 * v);
        #endif
        img[i] = (color_t) (255 * v);
    }
    f = fopen("out.raw", "wb");
    fwrite((void*) img, sizeof(color_t), 3 * num_pixels, f);
    fclose(f);

    return 0;
}
