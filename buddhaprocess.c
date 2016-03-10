#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

typedef unsigned char color_t;
typedef double bin_t;

color_t clamp(double v) {
    v = tanh(v * 0.004) * 255.0;
    if (v > 255) {
        return 255;
    }
    return (color_t) v;
}

int main() {
    int width = 1000;
    int height = 1000;
    int num_pixels = width * height;
    bin_t *rbins = calloc(num_pixels, sizeof(bin_t));
    bin_t *gbins = calloc(num_pixels, sizeof(bin_t));
    bin_t *bbins = calloc(num_pixels, sizeof(bin_t));

    FILE *f = fopen("bins.dat", "rb");
    assert(fread((void*) rbins, sizeof(bin_t), num_pixels, f));
    assert(fread((void*) gbins, sizeof(bin_t), num_pixels, f));
    assert(fread((void*) bbins, sizeof(bin_t), num_pixels, f));
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
