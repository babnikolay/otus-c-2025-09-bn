/*
Превращаем статичную картинку в анимацию, оборачиваем основной цикл вычислений в еще один цикл, который будет
постепенно уменьшать значение zoom. Каждому кадру даётся уникальное имя (например, frame_000.ppm).
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

void help(char *prog_name) {
    printf("\n");
    printf("=============== ПОМОЩЬ ===============\n");
    fprintf(stderr, "usage: %s degree frame_number zoom zoom_factor centerX centerY frequency phase_r phase_g phase_b\n", prog_name);
    printf("\n");
     printf("frame_number - Количество кадров для видео\n");
    printf("zoom         - Начальный масштаб, например, 2.5\n");
    printf("zoom_factor  - Коэффициент приближения (0.9 на 10%% каждый кадр)\n");
    printf("centerX      - Координата по оси X, по которой центрируется изображение\n");
    printf("centerY      - Координата по оси Y, по которой центрируется изображение\n");
    printf("frequency    - Частота. Определяет, как быстро меняется цвет при переходе от одной итерации к другой.\n");
    printf("               Низкая частота (например, 0.1): Цвет меняется медленно. Переходы будут широкими, плавными, \"растянутыми\".\n");
    printf("               Высокая частота (например, 1.0 или 2.0): Цвет меняется очень быстро.\n");
    printf("               Фрактал покроется множеством узких контрастных колец (эффект \"зебры\")\n");
    printf("               По умолчанию 2.0\n");
    printf("phase_r      - Фазовый сдвиг красного. Определяет баланс цветов\n");
    printf("phase_g      - Фазовый сдвиг зелёного. Определяет баланс цветов\n");
    printf("phase_b      - Фазовый сдвиг синего. Определяет баланс цветов\n");
    printf("\n");
    printf("Например:\n");
    printf("./mandelbrot_anim 1000 2.5 0.99 -1.768778833 -0.001738974 0.0 2.1 4.2\n");
    printf("======================================\n");
    printf("\n");
}

void generate_frame(int frame_num, double zoom, double frequency, double centerX, double centerY, double phase_r, double phase_g, double phase_b) {
    const int width = 800, height = 800;
    const int max_iter = 1000;

    // char filename[50];
    // sprintf(filename, "frames/frame_%05d.ppm", frame_num);

    // FILE *fp = fopen(filename, "wb");
    // if (!fp) {
    //     perror("Не удалось открыть файл");
    //     return;
    // }

    // fprintf(fp, "P6\n%d %d\n255\n", width, height);

    // Выделяем память под массив пикселей для PNG
    unsigned char *pixels = (unsigned char *)malloc(width * height * 3);
    if (!pixels) {
        fprintf(stderr, "Ошибка выделения памяти\n");
        return;
    }

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            double cx = centerX + (x - (double)width / 2.0) * (zoom / (double)width);
            double cy = centerY + (y - (double)height / 2.0) * (zoom / (double)height);
            // double cx = centerX + (x - (double)width / 3.0) * (zoom / (double)width);
            // double cy = centerY + (y - (double)height / 3.0) * (zoom / (double)height);

            double zx = 0.0, zy = 0.0;
            int iter = 0;

            while (zx*zx + zy*zy <= 4 && iter < max_iter) {
                double next_zx = zx * zx - zy * zy + cx;
                zy = 2 * zx * zy + cy;
                zx = next_zx;
                iter++;
            }

            unsigned char r = 255, g = 255, b = 255;

            if (iter < max_iter) {
                double r2 = zx * zx + zy * zy;
                // Логарифмические константы - используется для оптимизации log(log(sqrt(z)))
                const double inv_log2 = 1.0 / log(2.0);
                const double log_05 = log(0.5);
                double mu = iter + 1 - ((log_05 + log(log(r2))) * inv_log2);
                // double mu = iter + 1 - log(log(r2)) / log(2.0);

                r = (unsigned char)(sin(frequency * mu + phase_r) * 127 + 128);
                g = (unsigned char)(sin(frequency * mu + phase_g) * 127 + 128);
                b = (unsigned char)(sin(frequency * mu + phase_b) * 127 + 128);
            }

            // fputc(r, fp);
            // fputc(g, fp);
            // fputc(b, fp);

            int index = (y * width + x) * 3;
            pixels[index + 0] = (uint8_t)fminf(r, 255.0f);
            pixels[index + 1] = (uint8_t)fminf(g, 255.0f);
            pixels[index + 2] = (uint8_t)fminf(b, 255.0f);
        }
    }

    // fclose(fp);
    // printf("Кадр %05d готов (zoom: %e)\n", frame_num, zoom);

    char filename_png[60];
    sprintf(filename_png, "frames/mandelbrot_%05d.png", frame_num);

    // Сохранение массива в PNG
    if (stbi_write_png(filename_png, width, height, 3, pixels, width * 3)) {
        // printf("Фрактал успешно сохранен в %s\n", filename_png);
        printf("Кадр %05d готов (zoom: %e)\n", frame_num, zoom);
    } else {
        printf("Ошибка при сохранении PNG.\n");
    }

    free(pixels);

    return;
}

int main(int argc, char **argv) {
    // 1. Фиксируем время начала
    time_t start_time = time(NULL);
    printf("Начало: %s\n", ctime(&start_time));

    if (argc != 10) {
        char *prog_name = argv[0];
        help(prog_name);
        
        // 2. Фиксируем время окончания
        time_t end_time = time(NULL);
        printf("Окончание: %s\n", ctime(&end_time));

        // 3. Вычисляем разницу (длительность)
        double diff = difftime(end_time, start_time);
        printf("Программа работала %.0f сек.\n", diff);

        return 1;
    }

    int total_frames = atoi(argv[1]);    // Количество кадров для видео
    double current_zoom = atof(argv[2]); // Начальный масштаб, например, 2.5
    double zoom_factor = atof(argv[3]);  // Коэффициент приближения (0.9 на 10% каждый кадр, 0.99 - 1%)
    double frequency = atof(argv[4]);
    double centerX = atof(argv[5]);      // Координата по оси X, по которой центрируется изображение
    double centerY = atof(argv[6]);      // Координата по оси Y, по которой центрируется изображение
    double phase_r = atof(argv[7]);
    double phase_g = atof(argv[8]);
    double phase_b = atof(argv[9]);
    printf("Количество кадров для видео %d\n", total_frames);
    printf("Начальный масштаб %0.3f\n", current_zoom);
    printf("Коэффициент приближения %0.3f\n", zoom_factor);
    printf("Частота смены цветов %0.3f\n", frequency);
    /*
    Координаты цели (Долина Морских Коньков) Для степени 2
    double centerX = -0.743643887037158704752191506114774;
    double centerY = 0.131825904205311970493132056385139;
    */
    printf("Координата по оси X %0.3f\n", centerX);
    printf("Координата по оси Y %0.3f\n", centerY);
    printf("Фазовый сдвиг красного: %0.1f\n", phase_r);
    printf("Фазовый сдвиг зелёного: %0.1f\n", phase_g);
    printf("Фазовый сдвиг синего: %0.1f\n", phase_b);

    if (mkdir("frames", 0755) != 0) {
        if (errno == EEXIST) {
            printf("Каталог frames уже существует\n");
            return -1;
        }
        else {
            perror("Ошибка создания каталога\n");
            return -1;
        }
    }
    else {
        printf("Каталог 'frames' успешно создан.\n");
    }
    printf("\n");

    for (int i = 0; i < total_frames; i++) {
        generate_frame(i, current_zoom, frequency, centerX, centerY, phase_r, phase_g, phase_b);
        current_zoom *= zoom_factor;
    }

    // 2. Фиксируем время окончания
    time_t end_time = time(NULL);
    printf("Окончание: %s\n", ctime(&end_time));

    // 3. Вычисляем разницу (длительность)
    double diff = difftime(end_time, start_time);
    printf("Программа работала %.0f сек.\n", diff);

    return 0;
}
