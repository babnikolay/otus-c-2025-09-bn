/*
Превращаем статичную картинку в анимацию, оборачиваем основной цикл вычислений в еще один цикл, который будет
постепенно уменьшать значение zoom. Каждому кадру даётся уникальное имя (например, frame_000.ppm).
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>

void generate_frame(int frame_num, double zoom, double x, double y) {
    const int width = 800, height = 800;
    const int max_iter = 1000;

    double centerX = x;
    double centerY = y;

    char filename[30];
    sprintf(filename, "frames/frame_%05d.ppm", frame_num);
    FILE *fp = fopen(filename, "wb");
    if (!fp)
        return;

    fprintf(fp, "P6\n%d %d\n255\n", width, height);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            double ca = centerX + (x - width / 2.0) * (zoom / width);
            double cb = centerY + (y - height / 2.0) * (zoom / width);

            double a = 0, b = 0;
            int iter = 0;
            while (a * a + b * b <= 100 && iter < max_iter) {
                double next_a = a * a - b * b + ca;
                b = 2 * a * b + cb;
                a = next_a;
                iter++;
            }

            unsigned char r = 0, g = 0, bl = 0;
            if (iter < max_iter) {
                double modulus = sqrt(a * a + b * b);
                double mu = iter + 1 - log(log(modulus)) / log(2.0);
                r = (unsigned char)(sin(0.1 * mu + 0.0) * 127 + 128);
                g = (unsigned char)(sin(0.1 * mu + 2.0) * 127 + 128);
                bl = (unsigned char)(sin(0.1 * mu + 4.0) * 127 + 128);
            }
            fputc(r, fp);
            fputc(g, fp);
            fputc(bl, fp);
        }
    }

    fclose(fp);
    printf("Кадр %05d готов (zoom: %e)\n", frame_num, zoom);

    return;
}

int main(int argc, char **argv) {
    // 1. Фиксируем время начала
    time_t start_time = time(NULL);
    printf("Начало: %s\n", ctime(&start_time));

    if (argc != 6) {
        fprintf(stderr, "usage: %s frame_number zoom zoom_factor centerX centerY\n", argv[0]);
        printf("frame_number - Количество кадров для видео\n");
        printf("zoom         - Начальный масштаб, например, 2.5\n");
        printf("zoom_factor  - Коэффициент приближения (0.9 на 10%% каждый кадр)\n");
        printf("centerX      - Координата по оси X, по которой центрируется изображение\n");
        printf("centerY      - Координата по оси Y, по которой центрируется изображение\n");
        return 1;
    }

    printf("Количество кадров для видео %s\n", argv[1]);
    printf("Начальный масштаб %s\n", argv[2]);
    printf("Коэффициент приближения %s\n", argv[3]);
    /*
    Координаты цели (Долина Морских Коньков)
    double centerX = -0.743643887037158704752191506114774;
    double centerY = 0.131825904205311970493132056385139;
    */
    printf("Координата по оси X %s\n", argv[4]);
    printf("Координата по оси Y %s\n", argv[5]);

    int total_frames = atoi(argv[1]);    // Количество кадров для видео
    double current_zoom = atof(argv[2]); // Начальный масштаб, например, 2.5
    double zoom_factor = atof(argv[3]);  // Коэффициент приближения (0.9 на 10% каждый кадр)
    double centerX = atof(argv[4]);      // Координата по оси X, по которой центрируется изображение
    double centerY = atof(argv[5]);      // Координата по оси Y, по которой центрируется изображение

    if (mkdir("frames", 0755) != 0) {
        if (errno == EEXIST) {
            printf("Каталог уже существует\n");
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
        generate_frame(i, current_zoom, centerX, centerY);
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
