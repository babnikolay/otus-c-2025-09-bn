#include <stdio.h>
#include <math.h>

#define WIDTH 1000
#define HEIGHT 1000
#define MAX_ITER 256  // Количество итераций (влияет на детализацию и цвет)

int main() {
    FILE *fp = fopen("mandelbrot3.ppm", "wb");
    if (!fp) return 1;

    // Заголовок PPM: P6 (бинарный), ширина, высота, макс. значение цвета
    fprintf(fp, "P6\n%d %d\n255\n", WIDTH, HEIGHT);

    for (int py = 0; py < HEIGHT; py++) {
        for (int px = 0; px < WIDTH; px++) {
            
            // Центрируем и масштабируем (от -2.0 до 2.0)
            double cre = (px - WIDTH / 2.0) * 4.0 / WIDTH;
            double cim = (py - HEIGHT / 2.0) * 4.0 / HEIGHT;

            double x = 0.0, y = 0.0;
            int iter = 0;

            // Итерация z = z^3 + c
            while (x*x + y*y <= 4.0 && iter < MAX_ITER) {
                double x_next = x*x*x - 3.0*x*y*y + cre;
                double y_next = 3.0*x*x*y - y*y*y + cim;
                x = x_next;
                y = y_next;
                iter++;
            }

            unsigned char color = (iter == MAX_ITER) ? 0 : (iter % 255);

            // Записываем 3 байта (RGB) прямо в файл
            fputc(color, fp);
            fputc(color, fp);
            fputc(color, fp);
        }
    }

    fclose(fp);
    printf("Готово! Файл mandelbrot3.ppm создан.\n");

    return 0;
}
