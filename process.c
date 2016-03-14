#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include "buddhabrot.h"

typedef unsigned char color_t;

void get_layer(size_t iteration, size_t frame , size_t num_frames, FILE *f, bin_t *bins, int num_pixels) {
    fseek(f, (iteration * num_frames + frame) * num_pixels * sizeof(bin_t), SEEK_SET);
    assert(fread((void*) bins, sizeof(bin_t), num_pixels, f));
}

int main(int argc, char *argv[]) {
    if (argc < 8) {
        printf("Need width, height, layer, number of layers, frame, number of frames and normalizer.\n");
        return 1;
    }
    int width = atoi(argv[1]);
    int height = atoi(argv[2]);
    int layer = atoi(argv[3]);
    int num_layers = atoi(argv[4]);
    int frame = atoi(argv[5]);
    int num_frames = atoi(argv[6]);
    bin_t normalizer = atoi(argv[7]);
    int num_pixels = width * height;

    bin_t *bins = calloc(3 * num_pixels, sizeof(bin_t));
    bin_t *temp = malloc(num_pixels * sizeof(bin_t));

    FILE *f = fopen("buddhabulb_bins.dat", "rb");
    // for (int i = layer; i < num_layers; i++) {
    //     get_layer(i, frame, num_frames, f, temp, num_pixels);
    //     for (int j = 0; j < num_pixels; j++) {
    //         for (int c = 0; c < 3; c++) {
    //             if (c == 0) {
    //                 bins[3 * j + c] += temp[j] * (1 + exp(-0.007 * pow(i - 5, 2)) * 0.2);
    //             }
    //             if (c == 1) {
    //                 bins[3 * j + c] += temp[j] * exp(-0.007 * pow(i - 6, 2)) * 1.5;
    //             }
    //             if (c == 2) {
    //                 bins[3 * j + c] += temp[j] * (exp(-0.0006 * pow(i - 100, 2)) * 4.5 + i % 2);
    //             }
    //         }
    //     }
    // }

    get_layer(layer, frame, num_frames, f, temp, num_pixels);
    for (int j = 0; j < num_pixels; j++) {
        for (int c = 0; c < 3; c++) {
            bins[3 * j + c] += temp[j];
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

    if (!normalizer) {
        normalizer = bin_max;
    }

    color_t *img = malloc(3 * num_pixels * sizeof(color_t));
    for (int i = 0; i < 3 * num_pixels; i++) {
        double v = ((double) bins[i]) / normalizer;
        if (i % 3 == 2) {
            v = tanh(6 * (v - 0.66)) * 0.5 + 0.5;
        }
        else if (i % 3 == 1) {
            v = tanh(4 * (v - 0.33)) * 0.5 + 0.5;
        }
        else {
            v = tanh(4 * v);
        }
        img[i] = (color_t) (255 * v);
    }
    f = fopen("out.raw", "wb");
    fwrite((void*) img, sizeof(color_t), 3 * num_pixels, f);
    fclose(f);

    return 0;
}
