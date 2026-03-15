/*
Чтобы избавиться от "ступенчатости" (полос) и получить идеально плавные переходы, нужно использовать 
алгоритм - нормализованный счетчик итераций. Дробная часть добавляется к значению iter, основываясь на том,
насколько далеко "улетело" число  в момент выхода за радиус.
*/

#include <stdio.h>
#include <math.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

int main(int argc, char **argv) {
    if (argc != 6) {
        fprintf(stderr, "usage: %s degree frequency phase_r phase_g phase_b\n\n", argv[0]);
        printf("degree    - Степень уравнения Мандельброта, может быть дробной.\n");
        printf("frequency - Частота. Определяет, как быстро меняется цвет при переходе от одной итерации к другой.\n");
        printf("            Низкая частота (например, 0.1): Цвет меняется медленно. Переходы будут широкими, плавными, \"растянутыми\".\n");
        printf("            Высокая частота (например, 1.0 или 2.0): Цвет меняется очень быстро.\n");
        printf("            Фрактал покроется множеством узких контрастных колец (эффект \"зебры\").\n");
        printf("phase_r   - Фазовый сдвиг красного. Определяет баланс цветов.\n");
        printf("phase_g   - Фазовый сдвиг зелёного. Определяет баланс цветов.\n");
        printf("phase_b   - Фазовый сдвиг синего. Определяет баланс цветов.\n");
        printf("\n");
        printf("Например:\n");
        printf("./mandelbrot_smooth 3.5 0.1 0.0 2.1 4.2\n");
        printf("\n");
        
        return 1;
    }

    double degree = atof(argv[1]);
    double frequency = atof(argv[2]);
    double phase_r = atof(argv[3]);
    double phase_g = atof(argv[4]);
    double phase_b = atof(argv[5]);
    printf("Степень уравнения Мандельброта: %0.3f\n", degree);
    printf("Частота: %0.3f\n", frequency);
    printf("Фазовый сдвиг красного: %0.3f\n", phase_r);
    printf("Фазовый сдвиг зелёного: %0.3f\n", phase_g);
    printf("Фазовый сдвиг синего: %0.3f\n", phase_b);

    const int width = 1000, height = 1000;
    const int max_iter = 1000;
    const double radius_escape = 256.0; // Большой радиус для идеального сглаживания

    char filename[50];
    sprintf(filename, "mandelbrot_degree_%.3f.ppm", degree);

    FILE *fp = fopen(filename, "wb");
    if (!fp) return 1;

    fprintf(fp, "P6\n%d %d\n255\n", width, height);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Центрирование и масштабирование (диапазон примерно от -2 до 2)
            double cx = (x - width / 2.0) * 4.0 / width;
            double cy = (y - height / 2.0) * 4.0 / height;

            double zx = 0, zy = 0;
            int iter = 0;
            double modul = 0;

            // --- УНИВЕРСАЛЬНЫЙ ЦИКЛ ДЛЯ ЛЮБОЙ СТЕПЕНИ ---
            while (iter < max_iter) {
                modul = zx * zx + zy * zy;
                if (modul > radius_escape * radius_escape) break;

                // Переход в полярные координаты для возведения в степень n
                double r = sqrt(modul);
                double theta = atan2(zy, zx);
                
                /*
                Формула: z = z^n + c
                𝐱_𝐧𝐞𝐱𝐭=𝐫^𝐧⋅𝐜𝐨𝐬⁡(𝐧⋅𝛉)+𝐱_𝐢𝐧𝐢𝐭
                𝐲_𝐧𝐞𝐱𝐭=𝐫^𝐧⋅𝐬𝐢𝐧(𝐧⋅𝛉)+𝐲_𝐢𝐧𝐢𝐭
                */
                double r_pow = pow(r, degree);
                zx = r_pow * cos(degree * theta) + cx;
                zy = r_pow * sin(degree * theta) + cy;
                
                iter++;
            }

            unsigned char r_out = 0, g_out = 0, b_out = 0;

            if (iter < max_iter) {
                /*
                Алгоритм сглаживания: mu = n + 1 - log(log|z|) / log(degree)
                Формула mu: Вычисляет "дробную итерацию". 
                Теперь iter — это не целое число, а вещественное, что 
                позволяет цвету меняться непрерывно.
                Улучшенная формула сглаживания для произвольной степени n
                */
                double mu = iter + 1 - log(log(sqrt(zx*zx + zy*zy))) / log(degree);

                /*
                Создаем цикличный цветовой градиент на основе синусов sin(frequency * t + phase) для плавных цветовых переходов
                Это классический способ создать бесконечную "радугу", где фазовый сдвиг (числа 9.0, 15.0, 8.5) определяет баланс цветов
                Подбирая коэффициенты (9.0, 15.0, 8.5), можно менять палитру
                */
                r_out = (unsigned char)(sin(frequency * mu + phase_r) * 127 + 128);
                g_out = (unsigned char)(sin(frequency * mu + phase_g) * 127 + 128);
                b_out = (unsigned char)(sin(frequency * mu + phase_b) * 127 + 128);
            }

            fputc(r_out, fp);
            fputc(g_out, fp);
            fputc(b_out, fp);
        }
    }

    printf("Готово! Фрактал степени %.3f сохранен в %s\n", degree, filename);
    fclose(fp);

    return 0;
}
