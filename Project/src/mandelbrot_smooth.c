/*
Чтобы избавиться от "ступенчатости" (полос) и получить идеально плавные переходы, нужно использовать 
алгоритм - нормализованный счетчик итераций. Дробная часть добавляется к значению iter, основываясь на том,
насколько далеко "улетело" число  в момент выхода за радиус.
*/

#include <stdio.h>
#include <math.h>
// #include <stdlib.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

int main(int argc, char **argv)
{
    if (argc != 6) {
        fprintf(stderr, "usage: %s degree frequency kr kg kb\n\n", argv[0]);
        printf("degree    - Степень уравнения Мандельброта: 2 или 3\n");
        printf("frequency - Частота. Определяет, как быстро меняется цвет при переходе от одной итерации к другой\n");
        printf("            Низкая частота (например, 0.1): Цвет меняется медленно. Переходы будут широкими, плавными, \"растянутыми\".\n");
        printf("            Высокая частота (например, 1.0 или 2.0): Цвет меняется очень быстро.\n");
        printf("            Фрактал покроется множеством узких контрастных колец (эффект \"зебры\")\n");
        printf("phase_r   - Фазовый сдвиг красного. Определяет баланс цветов\n");
        printf("phase_g   - Фазовый сдвиг зелёного. Определяет баланс цветов\n");
        printf("phase_b   - Фазовый сдвиг синего. Определяет баланс цветов\n");
        printf("\n");
        printf("Например:\n");
        printf("./mandelbrot_smooth 3 0.1 0.0 2.1 4.2\n");
        printf("\n");
        return 1;
    }

    int degree = atoi(argv[1]);
    double frequency = atof(argv[2]);
    // double kr = 0.0, kg = 2.1, kb = 4.2;
    double phase_r = atof(argv[3]);
    double phase_g = atof(argv[4]);
    double phase_b = atof(argv[5]);
    printf("Степень уравнения Мандельброта: %d\n", degree);
    printf("Частота: %0.2f\n", frequency);
    printf("Фазовый сдвиг красного: %0.2f\n", phase_r);
    printf("Фазовый сдвиг зелёного: %0.2f\n", phase_g);
    printf("Фазовый сдвиг синего: %0.2f\n", phase_b);

    const int width = 1000, height = 1000;
    const int max_iter = 1000;

    char *filename;
    if (degree == 2) {
        filename = "mandelbrot_smooth_2.ppm";
    }
    else if (degree == 3) {
        filename = "mandelbrot_smooth_3.ppm";
    }
    else {
        printf("Степень уравнения Мандельброта некорректная\n");
        return 1;
    }

    FILE *fp = fopen(filename, "wb");
    if (!fp)
        return 0;

    fprintf(fp, "P6\n%d %d\n255\n", width, height);

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            double a = 0, b = 0;
            int iter = 0;

            if (degree == 2) {
                double ca = -2.0 + (x / (double)width) * 2.5;
                double cb = -1.25 + (y / (double)height) * 2.5;
                
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
            }
            else if (degree == 3) {
                double ca = (x - (double)width / 2.0) * 4.0 / (double)width;
                double cb = (y - (double)height / 2.0) * 4.0 / (double)height;

                // Итерация z = z^3 + c
                while (a*a + b*b <= 4.0 && iter < max_iter) {
                    double a_next = a*a*a - 3.0*a*b*b + ca;
                    double b_next = 3.0*a*a*b - b*b*b + cb;
                    a = a_next;
                    b = b_next;
                    iter++;
                }
            }

            unsigned char r = 0, g = 0, bl = 0;

            if (iter < max_iter)
            {
                /*
                Алгоритм сглаживания: mu = n + 1 - log(log|z|) / log(2)
                Формула mu: Вычисляет "дробную итерацию". 
                Теперь iter — это не целое число, а вещественное, что 
                позволяет цвету меняться непрерывно
                */
                double modulus = sqrt(a * a + b * b);
                double mu = iter + 1 - log(log(modulus) / log(2.0)) / log(3.0);

                /*
                Создаем цикличный цветовой градиент на основе синусов sin(frequency * t + phase) для плавных цветовых переходов
                Это классический способ создать бесконечную "радугу", где фазовый сдвиг (числа 9.0, 15.0, 8.5) определяет баланс цветов
                Подбирая коэффициенты (9.0, 15.0, 8.5), можно менять палитру
                */
                r = (unsigned char)(sin(frequency * mu + phase_r) * 127 + 128);
                g = (unsigned char)(sin(frequency * mu + phase_g) * 127 + 128);
                bl = (unsigned char)(sin(frequency * mu + phase_b) * 127 + 128);
            }

            fputc(r, fp);
            fputc(g, fp);
            fputc(bl, fp);
        }
    }

    printf("Готово! Плавный фрактал сохранен в %s\n", filename);
    fclose(fp);

    return 0;
}
