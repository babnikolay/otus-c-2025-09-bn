/*
Чтобы избавиться от "ступенчатости" (полос) и получить идеально плавные переходы, нужно использовать алгоритм
Renormalization (нормализованный счетчик итераций). дробную часть добавляется к значению iter, основываясь на том,
насколько далеко "улетело" число  в момент выхода за радиус.
*/

#include <stdio.h>
#include <math.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

int main()
{
    const int width = 1200, height = 1200;
    const int max_iter = 1000;

    FILE *fp = fopen("mandelbrot_smooth.ppm", "wb");
    if (!fp)
        return 0;

    fprintf(fp, "P6\n%d %d\n255\n", width, height);

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            double ca = -2.0 + (x / (double)width) * 2.5;
            double cb = -1.25 + (y / (double)height) * 2.5;

            double a = 0, b = 0;
            int iter = 0;

            while (a * a + b * b <= 100 && iter < max_iter)
            {
                /*
                Радиус увеличен с 2 до 10 для точности логарифма
                Это необходимо, чтобы логарифм в формуле сглаживания работал корректно и без искажений
                */
                double next_a = a * a - b * b + ca;
                b = 2 * a * b + cb;
                a = next_a;
                iter++;
            }

            unsigned char r = 0, g = 0, bl = 0;

            if (iter < max_iter)
            {
                /*
                Алгоритм сглаживания: mu = n + 1 - log(log|z|) / log(2)
                Формула mu: Вычисляет "дробную итерацию". Теперь iter — это не целое число, а вещественное,
                что позволяет цвету меняться непрерывно
                */
                double modulus = sqrt(a * a + b * b);
                double mu = iter + 1 - log(log(modulus)) / log(2.0);

                /*
                Создаем цикличный цветовой градиент на основе синусов sin(frequency * t + phase) для плавных цветовых переходов
                Это классический способ создать бесконечную "радугу", где фазовый сдвиг (числа 0.0, 2.0, 4.0) определяет баланс цветов
                Подбирая коэффициенты (0.9, 0.2, 0.5), можно менять палитру
                */
                r = (unsigned char)(sin(0.1 * mu + 1.8) * 127 + 128);
                g = (unsigned char)(sin(0.1 * mu + 5.0) * 127 + 128);
                bl = (unsigned char)(sin(0.1 * mu + 8.5) * 127 + 128);
            }

            fputc(r, fp);
            fputc(g, fp);
            fputc(bl, fp);
        }
    }

    fclose(fp);
    printf("Готово! Плавный фрактал сохранен в mandelbrot_smooth.ppm\n");

    return 0;
}
