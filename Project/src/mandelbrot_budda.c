#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <omp.h>
#include <time.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define WIDTH 800
#define HEIGHT 800
#define SAMPLES 40000000LL  // 40 млн для идеальной "графитовой" текстуры

// Разные лимиты итераций для каналов (R, G, B)
#define ITER_R 50000
#define ITER_G 5000
#define ITER_B 500

void render_channel(int *current_channel, int *r_buf, int *g_buf, int *b_buf, int max_iter, int draw_bg) {
    #pragma omp parallel
    {
        unsigned int seed = time(NULL) ^ omp_get_thread_num();
        #pragma omp for
        for (int s = 0; s < SAMPLES; s++) {
            double cr = (double)rand_r(&seed) / RAND_MAX * 4.0 - 2.0;
            double ci = (double)rand_r(&seed) / RAND_MAX * 4.0 - 2.0;

            double zr = 0, zi = 0;
            int escaped = 0;
            for (int i = 0; i < max_iter; i++) {
                double r2 = zr * zr, i2 = zi * zi;
                if (r2 + i2 > 4.0) { escaped = 1; break; }
                zi = 2.0 * zr * zi + ci;
                zr = r2 - i2 + cr;
            }

            if (escaped) {
                // Цветная туманность (только в текущий канал)
                zr = 0; zi = 0;
                for (int i = 0; i < max_iter; i++) {
                    double r2 = zr * zr, i2 = zi * zi;
                    if (r2 + i2 > 4.0) break;
                    zi = 2.0 * zr * zi + ci;
                    zr = r2 - i2 + cr;
                    int px = (int)((zr + 2.0) / 4.0 * WIDTH);
                    int py = (int)((zi + 2.0) / 4.0 * HEIGHT);
                    if (px >= 0 && px < WIDTH && py >= 0 && py < HEIGHT) {
                        #pragma omp atomic
                        current_channel[py * WIDTH + px]++;
                    }
                }
            } else if (draw_bg) {
                // СЕРЫЙ ФОН: записываем во ВСЕ каналы одинаково
                int px = (int)((cr + 2.0) / 4.0 * WIDTH);
                int py = (int)((ci + 2.0) / 4.0 * HEIGHT);

                if (px >= 0 && px < WIDTH && py >= 0 && py < HEIGHT) {
                    int idx = py * WIDTH + px;
                    #pragma omp atomic
                    r_buf[idx] += 1; // Одинаковое значение для R, G, B даст серый
                    #pragma omp atomic
                    g_buf[idx] += 1;
                    #pragma omp atomic
                    b_buf[idx] += 3;
                }
            }
        }
    }
}

int main() {
    // 1. Фиксируем время начала
    time_t start_time = time(NULL);
    printf("Начало: %s\n", ctime(&start_time));

    int *red = calloc(WIDTH * HEIGHT, sizeof(int));
    int *green = calloc(WIDTH * HEIGHT, sizeof(int));
    int *blue = calloc(WIDTH * HEIGHT, sizeof(int));

    double start, end;

    printf("Rendering R (long paths)...\n");
    start = omp_get_wtime();
    // Третий аргумент (1) означает, что при расчете Red мы также рисуем серый силуэт
    render_channel(red, red, green, blue, ITER_R, 1);
    end = omp_get_wtime();
    printf("R channel done in %.2f seconds.\n", end - start);

    printf("Rendering G (medium paths)...\n");
    start = omp_get_wtime();
    
    render_channel(green, red, green, blue, ITER_G, 0); 
    end = omp_get_wtime();
    printf("G channel done in %.2f seconds.\n", end - start);

    printf("Rendering B (short paths)...\n");
    start = omp_get_wtime();
    render_channel(blue, red, green, blue, ITER_B, 0);
    end = omp_get_wtime();
    printf("B channel done in %.2f seconds.\n", end - start);

    // Сохраняем в формат PNG
    const int channels = 3; // RGB

    // Выделяем память под массив пикселей для PNG
    unsigned char *pixels = (unsigned char *)malloc(WIDTH * HEIGHT * channels);
    if (!pixels) {
        fprintf(stderr, "Ошибка выделения памяти\n");
        return 1;
    }

    // Подбор делителя для яркости (можно покрутить 100 на 50 или 200)
    // double exposure = SAMPLES / 100.0;

    // Находим максимум для каждого канала (упрощенно)
    // В идеале нужно найти max отдельно для R, G и B для баланса белого
    for (int i = 0; i < WIDTH * HEIGHT; i++) {
        // Логарифмическое масштабирование и запись RGB байтов
        // unsigned char r = (unsigned char)(255 * log1p(red[i]) / log1p(exposure)); // Примерный делитель
        // unsigned char g = (unsigned char)(255 * log1p(green[i]) / log1p(exposure));
        // unsigned char b = (unsigned char)(255 * log1p(blue[i]) / log1p(exposure));

        /*
        Логарифмическая яркость (коэффициент 12.0 - 15.0 для графита)
        Коэффициент 0.6 подтянет тени, делая графит видимым, но не серым
        */
        float k = 15.0f;
        // Применяем log1p для сжатия динамического диапазона (ln(1 + x))
        float r = log1pf((float)red[i])   * k;
        float g = log1pf((float)green[i]) * k;
        float b = log1pf((float)blue[i])  * k;

        // pixels[i * 3 + 0] = r;
        // pixels[i * 3 + 1] = g;
        // pixels[i * 3 + 2] = b;

        // Ограничение и запись в массив пикселей
        pixels[i * 3 + 0] = (uint8_t)fminf(r, 255.0f);
        pixels[i * 3 + 1] = (uint8_t)fminf(g, 255.0f);
        pixels[i * 3 + 2] = (uint8_t)fminf(b, 255.0f);
    }

    char filename_png[60];
    sprintf(filename_png, "mandelbrot_budda.png");

    // Сохранение массива в PNG
    if (stbi_write_png(filename_png, WIDTH, HEIGHT, channels, pixels, WIDTH * channels)) {
        printf("Фрактал успешно сохранен в %s\n", filename_png);
    } else {
        printf("Ошибка при сохранении PNG\n");
    }

    free(pixels);
    free(red); free(green); free(blue);
    
    // 2. Фиксируем время окончания
    time_t end_time = time(NULL);
    printf("Окончание: %s\n", ctime(&end_time));

    // 3. Вычисляем разницу (длительность)
    double diff = difftime(end_time, start_time);
    printf("Программа работала %.0f сек.\n", diff);

    return 0;
}
