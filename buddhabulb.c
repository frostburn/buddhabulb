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

int to_index(double x, double y, double z, int width, int height) {
    double t = x;
    x = 0.62348980185873359 * x - 0.7818314824680298 * z;
    z = 0.62348980185873359 * z + 0.7818314824680298 * t;
    t = y;
    y = 0.95557280578614068 * y - 0.29475517441090421 * z;
    // z = 0.95557280578614068 * z + 0.29475517441090421 * t;

    x = (x + 1.5) * 0.3333;
    y = (y + 1.5) * 0.3333;
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
    if (argc < 3) {
        printf("Need width, height and number of layers.\n");
        return 1;
    }
    int width = atoi(argv[1]);
    int height = atoi(argv[2]);
    size_t num_layers = atoi(argv[3]);
    size_t num_pixels = width * height;
    size_t num_bins = num_pixels * num_layers;
    bin_t *bins = calloc(num_bins, sizeof(bin_t));

    double bailout = 4.0;
    size_t max_iter = 2000;
    size_t save_interval = 50000000;

    size_t num_symmetries = 2;  // Flip y-axis
    num_symmetries *= 5;  // Rotational symmetry when n=6
    int *trajectory = malloc(num_symmetries * max_iter * sizeof(int));
    srand(time(0));
    size_t i = 0;

    FILE *f;

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
        // Check main bulb.
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
                        int index = trajectory[num_symmetries * k + s];
                        if (index >= 0) {
                            index += k * num_pixels;
                            if (index < num_bins && bins[index] < BIN_MAX) {
                                bins[index]++;
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

            trajectory[num_symmetries * j] = to_index(x, y, z, width, height);
            trajectory[num_symmetries * j + 1] = to_index(x, -y, z, width, height);
            double tx = 0.30901699437494745 * x - 0.95105651629515353 * y;
            double ty = 0.30901699437494745 * y + 0.95105651629515353 * x;
            trajectory[num_symmetries * j + 2] = to_index(tx, ty, z, width, height);
            trajectory[num_symmetries * j + 3] = to_index(tx, -ty, z, width, height);
            double ttx = tx;
            tx = 0.30901699437494745 * tx - 0.95105651629515353 * ty;
            ty = 0.30901699437494745 * ty + 0.95105651629515353 * ttx;
            trajectory[num_symmetries * j + 4] = to_index(tx, ty, z, width, height);
            trajectory[num_symmetries * j + 5] = to_index(tx, -ty, z, width, height);
            ttx = tx;
            tx = 0.30901699437494745 * tx - 0.95105651629515353 * ty;
            ty = 0.30901699437494745 * ty + 0.95105651629515353 * ttx;
            trajectory[num_symmetries * j + 6] = to_index(tx, ty, z, width, height);
            trajectory[num_symmetries * j + 7] = to_index(tx, -ty, z, width, height);
            ttx = tx;
            tx = 0.30901699437494745 * tx - 0.95105651629515353 * ty;
            ty = 0.30901699437494745 * ty + 0.95105651629515353 * ttx;
            trajectory[num_symmetries * j + 8] = to_index(tx, ty, z, width, height);
            trajectory[num_symmetries * j + 9] = to_index(tx, -ty, z, width, height);
        }
    }

    return 0;
}
