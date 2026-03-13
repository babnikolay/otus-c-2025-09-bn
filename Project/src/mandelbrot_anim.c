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

void generate_frame(int frame_num, double zoom, double x, double y, double degree, 
                    double frequency, double phase_r, double phase_g, double phase_b) {
    const int width = 800, height = 800;
    const int max_iter = 1000;
    const double radius_escape = 256.0; // Большой радиус для идеального сглаживания

    double centerX = x;
    double centerY = y;

    char filename[50];
    sprintf(filename, "frames/frame_%g_%05d.ppm", degree, frame_num);
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
            double modulus = 0;

            // --- УНИВЕРСАЛЬНЫЙ ЦИКЛ ДЛЯ ЛЮБОЙ СТЕПЕНИ ---
            while (iter < max_iter) {
                modulus = a * a + b * b;
                if (modulus > radius_escape * radius_escape) break;

                // Переход в полярные координаты для возведения в степень n
                double r = sqrt(modulus);
                double theta = atan2(b, a);
                
                // Формула: z = z^n + c
                double r_pow = pow(r, degree);
                a = r_pow * cos(degree * theta) + ca;
                b = r_pow * sin(degree * theta) + cb;
                
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
                double mu = iter + 1 - log(log(sqrt(a*a + b*b))) / log(degree);

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

    fclose(fp);
    printf("Кадр %05d готов (zoom: %e)\n", frame_num, zoom);

    return;
}

int main(int argc, char **argv) {
    // 1. Фиксируем время начала
    time_t start_time = time(NULL);
    printf("Начало: %s\n", ctime(&start_time));

    if (argc != 11) {
        fprintf(stderr, "usage: %s degree frame_number zoom zoom_factor centerX centerY frequency phase_r phase_g phase_b\n", argv[0]);
        printf("\n");
        printf("degree       - Степень уравнения Мандельброта: 2, 3.5 или другие\n");
        printf("frame_number - Количество кадров для видео\n");
        printf("zoom         - Начальный масштаб, например, 2.5\n");
        printf("zoom_factor  - Коэффициент приближения (0.9 на 10%% каждый кадр)\n");
        printf("centerX      - Координата по оси X, по которой центрируется изображение\n");
        printf("centerY      - Координата по оси Y, по которой центрируется изображение\n");
        printf("frequency    - Частота. Определяет, как быстро меняется цвет при переходе от одной итерации к другой\n");
        printf("                Низкая частота (например, 0.1): Цвет меняется медленно. Переходы будут широкими, плавными, \"растянутыми\".\n");
        printf("                Высокая частота (например, 1.0 или 2.0): Цвет меняется очень быстро.\n");
        printf("                Фрактал покроется множеством узких контрастных колец (эффект \"зебры\")\n");
        printf("phase_r      - Фазовый сдвиг красного. Определяет баланс цветов\n");
        printf("phase_g      - Фазовый сдвиг зелёного. Определяет баланс цветов\n");
        printf("phase_b      - Фазовый сдвиг синего. Определяет баланс цветов\n");
        printf("\n");
        printf("Например:\n");
        printf("./mandelbrot_anim 2.5 1000 2.5 0.99 -1.768778833 -0.001738974 0.3 0.0 2.1 4.2\n");
        printf("\n");
        
        // 2. Фиксируем время окончания
        time_t end_time = time(NULL);
        printf("Окончание: %s\n", ctime(&end_time));

        // 3. Вычисляем разницу (длительность)
        double diff = difftime(end_time, start_time);
        printf("Программа работала %.0f сек.\n", diff);

        return 1;
    }

    double degree = atof(argv[1]);
    int total_frames = atoi(argv[2]);    // Количество кадров для видео
    double current_zoom = atof(argv[3]); // Начальный масштаб, например, 2.5
    double zoom_factor = atof(argv[4]);  // Коэффициент приближения (0.9 на 10% каждый кадр)
    double centerX = atof(argv[5]);      // Координата по оси X, по которой центрируется изображение
    double centerY = atof(argv[6]);      // Координата по оси Y, по которой центрируется изображение
    double frequency = atof(argv[7]);
    double phase_r = atof(argv[8]);
    double phase_g = atof(argv[9]);
    double phase_b = atof(argv[10]);
    printf("Степень уравнения Мандельброта: %0.3f\n", degree);
    printf("Количество кадров для видео %d\n", total_frames);
    printf("Начальный масштаб %0.3f\n", current_zoom);
    printf("Коэффициент приближения %0.3f\n", zoom_factor);
    /*
    Координаты цели (Долина Морских Коньков)
    double centerX = -0.743643887037158704752191506114774;
    double centerY = 0.131825904205311970493132056385139;
    */
    printf("Координата по оси X %0.3f\n", centerX);
    printf("Координата по оси Y %0.3f\n", centerY);
    printf("Частота: %0.1f\n", frequency);
    printf("Фазовый сдвиг красного: %0.1f\n", phase_r);
    printf("Фазовый сдвиг зелёного: %0.1f\n", phase_g);
    printf("Фазовый сдвиг синего: %0.1f\n", phase_b);

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
        generate_frame(i, current_zoom, centerX, centerY, degree, frequency, phase_r, phase_g, phase_b);
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
