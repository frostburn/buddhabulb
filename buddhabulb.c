#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <limits.h>
#include "buddhabrot.h"

double uniform() {
    return 2 * (((double) rand()) / RAND_MAX) - 1;
}

static double *COSINES;
static double *SINES;

int to_index(double x, double y, double z, int frame, int width, int height) {
    double c = COSINES[frame];
    double s = SINES[frame];
    double temp = x;
    x = c * x - s * z;
    z = c * z + s * temp;
    y = 0.95557280578614068 * y - 0.29475517441090421 * z;

    x = (x + 1.5) * 0.333333;
    y = (y + 1.5) * 0.333333;
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

int main(int argc, char *argv[]) {
    if (argc < 5) {
        printf("Need width, height, number of layers and number of frames.\n");
        return 1;
    }
    int width = atoi(argv[1]);
    int height = atoi(argv[2]);
    size_t num_layers = atoi(argv[3]);
    size_t num_frames = atoi(argv[4]);
    size_t num_pixels = width * height;
    size_t num_bins = num_pixels * num_layers * num_frames;
    bin_t *bins = calloc(num_bins, sizeof(bin_t));

    FILE *f;
    if (argc == 6) {
        f = fopen("buddhabulb_bins.dat", "rb");
        assert(fread((void*) bins, sizeof(bin_t), num_bins, f));
        fclose(f);
        printf("Load done\n");

        // bin_t *temp = malloc(num_bins * sizeof(bin_t));
        // f = fopen("buddhabulb_bins2.dat", "rb");
        // assert(fread((void*) temp, sizeof(bin_t), num_bins, f));
        // fclose(f);
        // for (size_t i = 0; i < num_bins; i++) {
        //     bins[i] += temp[i];
        // }
        // f = fopen("buddhabulb_bins.dat", "wb");
        // fwrite((void*) bins, sizeof(bin_t), num_bins, f);
        // fclose(f);
        // printf("Merge done\n");
        // return 0;
    }

    COSINES = malloc(sizeof(double) * num_frames);
    SINES = malloc(sizeof(double) * num_frames);

    for (int frame = 0; frame < num_frames; frame++) {
        COSINES[frame] = cos(1 + (2 * M_PI * frame) / num_frames);
        SINES[frame] = sin(1 + (2 * M_PI * frame) / num_frames);
    }

    double bailout = 4.0;
    size_t max_iter = 2000;
    size_t save_interval = 5000000;

    size_t num_symmetries = 1;
    num_symmetries *= 2;  // Flip y-axis
    num_symmetries *= 5;  // Rotational symmetry when n=6
    int *trajectory = malloc(num_frames * num_symmetries * max_iter * sizeof(int));
    srand(time(0));
    size_t i = 0;

    while(1) {
        if (i % save_interval == save_interval - 1) {
            f = fopen("buddhabulb_bins.dat", "wb");
            fwrite((void*) bins, sizeof(bin_t), num_bins, f);
            fclose(f);
            printf("Save done\n");
        }
        i++;

        double cx = 4 * uniform();
        double cy = 4 * uniform();
        double cz = 4 * uniform();
        // Check main bulb n=6.
        if (cx*cx + cy*cy + cz*cz < 0.345) {
            continue;
        }
        double x = 0.0;
        double y = 0.0;
        double z = 0.0;
        for (size_t j = 0; j < max_iter; j++) {
            double x2 = x*x;
            double y2 = y*y;
            double rxy = x2 + y2;
            double r = rxy + z*z;
            if (r > bailout) {
                for (size_t k = 0; k < j; k++) {
                    for (size_t s = 0; s < num_symmetries; s++) {
                        for (size_t frame = 0; frame < num_frames; frame++) {
                            size_t idx = num_symmetries * (frame + num_frames * j);
                            int index = trajectory[idx + s];
                            if (index >= 0) {
                                index += (k * num_frames + frame) * num_pixels ;
                                if (index < num_bins && bins[index] < BIN_MAX) {
                                    bins[index]++;
                                }
                            }
                        }
                    }
                }
                break;
            }

            // Special case for n=6
            double x4 = x2 * x2;
            double x6 = x2 * x4;
            double y4 = y2 * y2;
            double y6 = y2 * y4;
            double z2 = z * z;
            double z4 = z2 * z2;
            if (rxy > 0.000001) {
                double q = pow(rxy, -2.5);
                y = 4*x*y*(3*x4 - 10*x2*y2 + 3*y4)*z*(x2 + y2 - 3*z2)*(3*(x2 + y2) - z2) * q + cy;
                x = 2*(x6 - 15*x4*y2 + 15*x2*y4 - y6)*z*(x2 + y2 - 3*z2)*(3*(x2 + y2) - z2) * q + cx;
            }
            else {
                y = cy;
                x = cx;
            }
            z = -(rxy - z2)*(rxy * (rxy - 14*z2) + z4) + cz;

            // General formula
            // r *= r * r;
            // double phi = atan2(y, x);
            // double theta = atan2(sqrt(rxy), z);
            // double x = r * sin(6 * theta);
            // double y = x * sin(6 * phi) + cy;
            // x = x * cos(6 * phi) + cx;
            // z = r * cos(6 * theta) + cz;

            for (size_t frame = 0; frame < num_frames; frame++) {
                size_t idx = num_symmetries * (frame + num_frames * j);
                trajectory[idx] = to_index(x, y, z, frame, width, height);
                trajectory[idx + 1] = to_index(x, -y, z, frame, width, height);
                double tx = 0.30901699437494745 * x - 0.95105651629515353 * y;
                double ty = 0.30901699437494745 * y + 0.95105651629515353 * x;
                double mx = 0.30901699437494745 * x + 0.95105651629515353 * y;
                double my = -0.30901699437494745 * y + 0.95105651629515353 * x;
                trajectory[idx + 2] = to_index(tx, ty, z, frame, width, height);
                trajectory[idx + 3] = to_index(mx, my, z, frame, width, height);
                double temp = tx;
                tx = 0.30901699437494745 * tx - 0.95105651629515353 * ty;
                ty = 0.30901699437494745 * ty + 0.95105651629515353 * temp;
                temp = mx;
                tx = 0.30901699437494745 * mx - 0.95105651629515353 * my;
                ty = 0.30901699437494745 * my + 0.95105651629515353 * temp;
                trajectory[idx + 4] = to_index(tx, ty, z, frame, width, height);
                trajectory[idx + 5] = to_index(mx, my, z, frame, width, height);
                temp = tx;
                tx = 0.30901699437494745 * tx - 0.95105651629515353 * ty;
                ty = 0.30901699437494745 * ty + 0.95105651629515353 * temp;
                temp = mx;
                tx = 0.30901699437494745 * mx - 0.95105651629515353 * my;
                ty = 0.30901699437494745 * my + 0.95105651629515353 * temp;
                trajectory[idx + 6] = to_index(tx, ty, z, frame, width, height);
                trajectory[idx + 7] = to_index(mx, my, z, frame, width, height);
                temp = tx;
                tx = 0.30901699437494745 * tx - 0.95105651629515353 * ty;
                ty = 0.30901699437494745 * ty + 0.95105651629515353 * temp;
                temp = mx;
                tx = 0.30901699437494745 * mx - 0.95105651629515353 * my;
                ty = 0.30901699437494745 * my + 0.95105651629515353 * temp;
                trajectory[idx + 8] = to_index(tx, ty, z, frame, width, height);
                trajectory[idx + 9] = to_index(mx, my, z, frame, width, height);
            }
        }
    }

    return 0;
}
