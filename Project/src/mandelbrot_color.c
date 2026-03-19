#include <stdio.h>
// #include <stdlib.h>
#include <math.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

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

    if (degree != 2 && degree != 3) {
        printf("Степень уравнения Мандельброта некорректная. Может быть только 2 или 3.\n");
        return 1;
    }

    const int width = 1000, height = 1000;
    const int max_iter = 512; // Для цвета лучше брать не слишком большое число
    const int channels = 3; // RGB

    // Выделяем память под массив пикселей для PNG
    unsigned char *image_data = (unsigned char *)malloc(width * height * channels);
    if (!image_data) {
        fprintf(stderr, "Ошибка выделения памяти\n");
        return 1;
    }

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {

            double zx = 0, zy = 0;
            int iter = 0;

            if (degree == 2) {
                double cx = -2.0 + ((double)x / (double)width) * 3.0;
                double cy = -1.5 + ((double)y / (double)height) * 3.0;

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
                double cx = ((double)x - (double)width / 2.0) * 4.0 / (double)width;
                double cy = ((double)y - (double)height / 2.0) * 4.0 / (double)height;

                // Итерация z = z^3 + c
                while (zx*zx + zy*zy <= 4.0 && iter <= max_iter) {
                    double zx_next = zx*zx*zx - 3.0*zx*zy*zy + cx;
                    double zy_next = 3.0*zx*zx*zy - zy*zy*zy + cy;
                    zx = zx_next;
                    zy = zy_next;
                    iter++;
                }
            }

            unsigned char r, g, b;

            if (iter > max_iter) {
                // Точки внутри множества всегда черные
                r = g = b = 0;
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
                b = (unsigned char)(8.5 * (1 - t) * (1 - t) * (1 - t) * t * 255);
            }

            // Запись в массив для PNG
            int index = (y * width + x) * channels;
            image_data[index]     = r;
            image_data[index + 1] = g;
            image_data[index + 2] = b;
        }
    }

    char filename_png[60];
    sprintf(filename_png, "mandelbrot_degree_%d.png", degree);

    // Сохранение массива в PNG
    if (stbi_write_png(filename_png, width, height, channels, image_data, width * channels)) {
        printf("Фрактал успешно сохранен в %s\n", filename_png);
    } else {
        printf("Ошибка при сохранении PNG\n");
    }

    free(image_data);

    return 0;
}
