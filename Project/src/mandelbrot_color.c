#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s degree\n\n", argv[0]);
        printf("degree - Степень уравнения Мандельброта: 2 или 3\n");
        printf("\n");
        printf("Например:\n");
        printf("./mandelbrot_color 3\n");
        printf("\n");
        return 1;
    }

    printf("Степень уравнения Мандельброта: %s\n", argv[1]);
    int degree = atoi(argv[1]);

    const int width = 1000, height = 1000;
    const int max_iter = 512; // Для цвета лучше брать не слишком большое число
    
    char *filename;
    if (degree == 2) {
        filename = "mandelbrot_color_2.ppm";
    }
    else if (degree == 3) {
        filename = "mandelbrot_color_3.ppm";
    }
    else {
        printf("Степень уравнения Мандельброта некорректная\n");
        return 1;
    }

    FILE *fp = fopen(filename, "wb");

    fprintf(fp, "P6\n%d %d\n255\n", width, height);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {

            double zx = 0, zy = 0;
            int iter = 0;

            if (degree == 2) {
                double cx = -2.0 + (x / (double)width) * 2.5;
                double cy = -1.25 + (y / (double)height) * 2.5;

                // Вычисление следующих значений комплексного числа z по формуле: z(n+1) = z(n)^2 + c
                while (zx * zx + zy * zy <= 4 && iter < max_iter)
                {
                    double next_zx = zx * zx - zy * zy + cx;
                    zy = 2 * zx * zy + cy;
                    zx = next_zx;
                    iter++;
                }
            }
            else if (degree == 3) {
                double cx = (x - (double)width / 2.0) * 4.0 / (double)width;
                double cy = (y - (double)height / 2.0) * 4.0 / (double)height;

                // Итерация z = z^3 + c
                while (zx*zx + zy*zy <= 4.0 && iter < max_iter) {
                    double zx_next = zx*zx*zx - 3.0*zx*zy*zy + cx;
                    double zy_next = 3.0*zx*zx*zy - zy*zy*zy + cy;
                    zx = zx_next;
                    zy = zy_next;
                    iter++;
                }
            }

            unsigned char r, g, b_col;

            if (iter == max_iter) {
                // Точки внутри множества всегда черные
                r = g = b_col = 0;
            }
            else {
                // Коэффициент для плавности (от 0 до 1)
                double t = (double)iter / max_iter;

                /*
                Математическая раскраска через цвета задаётся через полиномиальные выражения вида:
                P(t) = C * (1−t)^zx * t^zy, где zx + zy = 4 (степень 4), а C — константа-множитель для настройки яркости,
                которые создают плавные градиенты с помощью полиномиальных функций, близких к базису Бернштейна.
                Множители (9, 15, 8.5) можно менять для поиска других оттенков
                */
                r = (unsigned char)(9.0 * (1 - t) * t * t * t * 255);
                g = (unsigned char)(15.0 * (1 - t) * (1 - t) * t * t * 255);
                b_col = (unsigned char)(8.5 * (1 - t) * (1 - t) * (1 - t) * t * 255);
            }

            fputc(r, fp);
            fputc(g, fp);
            fputc(b_col, fp);
        }
    }

    fclose(fp);
    printf("Цветное изображение создано!\n");

    return 0;
}
