#include <stdio.h>

#define WIDTH 100
#define HEIGHT 40
#define MAX_ITER 1000

int mandelbrot(double real, double imag) {
    double z_real = 0, z_imag = 0;
    int iter = 0;

    while (z_real * z_real + z_imag * z_imag <= 4 && iter < MAX_ITER) {
        double temp = z_real * z_real - z_imag * z_imag + real;
        z_imag = 2 * z_real * z_imag + imag;
        z_real = temp;
        iter++;
    }
    return iter;
}

int main() {
    for (int y = 0; y < HEIGHT; y++) {
        double imag = (y - HEIGHT / 2.0) * 3.0 / HEIGHT;
        for (int x = 0; x < WIDTH; x++) {
            double real = (x - WIDTH / 2.0) * 3.5 / WIDTH;
            int iter = mandelbrot(real, imag);

            if (iter == MAX_ITER)
                printf("#");  // Точка принадлежит множеству
            else if (iter > MAX_ITER / 2)
                printf("+");
            else if (iter > MAX_ITER / 10)
                printf(".");
            else
                printf(" ");
        }
        printf("\n");
    }
    return 0;
}
