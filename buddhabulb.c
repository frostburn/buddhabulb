#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <unistd.h>

typedef unsigned char color_t;
typedef double bin_t;

double uniform() {
    return 2 * (((double) rand()) / RAND_MAX) - 1;
}

int to_index(double x, double y, double z, int width, int height) {
    x += 0.15 * z;
    y += 0.15 * z;
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

color_t clamp(double v) {
    v *= 1;
    if (v > 255) {
        return 255;
    }
    return (color_t) v;
}

int main() {
    int width = 100;
    int height = 100;
    int num_pixels = width * height;
    bin_t *rbins = calloc(num_pixels, sizeof(bin_t));
    bin_t *gbins = calloc(num_pixels, sizeof(bin_t));
    bin_t *bbins = calloc(num_pixels, sizeof(bin_t));

    double bailout = 4.0;
    size_t max_iter = 2000;
    size_t samples = 10000000ULL;
    int num_symmetries = 2;  // Flip y-axis
    num_symmetries *= 5;  // Rotational symmetry when n=6
    int *trajectory = malloc(num_symmetries * max_iter * sizeof(int));
    srand(time(0));
    for (size_t i = 0; i < samples; i++) {
        if (i % 1000000 == 0) {
            printf("%zu\n", (i * 100) / samples);
        }
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
                for (size_t k = 3; k < j; k++) {
                    if (k % 3 != 0) {
                        continue;
                    }
                    for (size_t s = 0; s < num_symmetries; s++) {
                        int index = trajectory[num_symmetries * k + s];
                        if (index >= 0) {
                            rbins[index] += exp(-0.0001*(k-15)*(k-15)) * 0.5;
                            gbins[index] += exp(-0.0001*(k-300)*(k-300));
                            bbins[index] += exp(-0.0001*(k-600)*(k-600)) * 4;
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

    FILE *f = fopen("bins.dat", "wb");
    fwrite((void*) rbins, sizeof(bin_t), num_pixels, f);
    fwrite((void*) gbins, sizeof(bin_t), num_pixels, f);
    fwrite((void*) bbins, sizeof(bin_t), num_pixels, f);
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
