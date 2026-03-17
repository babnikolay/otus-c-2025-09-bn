/*
Чтобы избавиться от "ступенчатости" (полос) и получить идеально плавные переходы, нужно использовать 
алгоритм - нормализованный счетчик итераций. Дробная часть добавляется к значению iter, основываясь на том,
насколько далеко "улетело" число  в момент выхода за радиус.
*/
#include <stdio.h>
#include <math.h>
#include <time.h>
// #include <complex.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

typedef struct {
    unsigned char r, g, b;
} Color;

Color get_color(int iter, int max_iter, double zx, double zy, double degree,
                double frequency, double phase_r, double phase_g, double phase_b) {
    if (iter >= max_iter) 
        return (Color){0, 0, 0};

    /*
    Алгоритм сглаживания: mu = n + 1 - log(log|z|) / log(degree)
    Формула mu: Вычисляет "дробную итерацию". 
    Теперь n — это не целое число, а вещественное, что позволяет цвету меняться непрерывно.
    Улучшенная формула сглаживания для произвольной степени n
    */
    double mu;
    // Защита от d=1 и d=0
    if (fabs(degree - 1.0) < 0.001 || degree < 0.001) {
        mu = (double)iter; 
    } else {
        double modul = sqrt(zx * zx + zy * zy);
        // Защита log(log) — значение внутри должно быть > 1
        if (modul > 1.001) {
            mu = (double)iter + 1.0 - log(log(modul)) / log(degree);
        } else {
            mu = (double)iter;
        }
    }

    if (isnan(mu) || isinf(mu)) mu = (double)iter;

    Color c;

    /*
    Создаем цикличный цветовой градиент на основе синусов sin(frequency * t + phase) для плавных цветовых переходов
    Это классический способ создать бесконечную "радугу", где фазовый сдвиг (числа phase_r, phase_g, phase_b) определяет баланс цветов
    Формула: 0.5 + 0.5 * sin(f * t + phase) масштабирует синус в диапазон [0, 1]
    Подбирая коэффициенты (phase_r=9.0, phase_g=15.0, phase_b=8.5), можно менять палитру
    */
    c.r = (unsigned char)(255 * (0.5 + 0.5 * sin(frequency * mu + phase_r)));
    c.g = (unsigned char)(255 * (0.5 + 0.5 * sin(frequency * mu + phase_g)));
    c.b = (unsigned char)(255 * (0.5 + 0.5 * sin(frequency * mu + phase_b)));

    return c;
}

int main(int argc, char **argv) {
    // 1. Фиксируем время начала
    time_t start_time = time(NULL);
    printf("Начало: %s\n", ctime(&start_time));

    if (argc != 6) {
        fprintf(stderr, "usage: %s degree frequency phase_r phase_g phase_b\n\n", argv[0]);
        printf("degree    - Степень уравнения Мандельброта, может быть дробной.\n");
        printf("frequency - Частота. Определяет, как быстро меняется цвет при переходе от одной итерации к другой.\n");
        printf("            !!! Не должна быть равна 0.\n");
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
    printf("Степень уравнения Мандельброта: %0.5f\n", degree);
    printf("Частота: %0.5f\n", frequency);
    printf("Фазовый сдвиг красного: %0.3f\n", phase_r);
    printf("Фазовый сдвиг зелёного: %0.3f\n", phase_g);
    printf("Фазовый сдвиг синего: %0.3f\n", phase_b);

    /*
    Большой радиус для идеального сглаживания
    Порог выхода (для d < 0 лучше брать больше)
    */ 
    double radius_escape = 100.0;
    int width = 1000, height = 1000;
    int max_iter = 100;
    const int channels = 3; // RGB

    // Параметры камеры (можно менять в реальном времени)
    double zoom = 4.0;     // Чем больше число, тем дальше камера (для d=0.5 и ниже должно быть порядка 10-20)
    double offsetX = 0.0;  // Смещение по горизонтали
    double offsetY = 0.0;   // Смещение по вертикали (0 — центр)

    // Выделяем память под массив пикселей для PNG
    unsigned char *image_data = (unsigned char *)malloc(width * height * channels);
    if (!image_data) {
        fprintf(stderr, "Ошибка выделения памяти\n");
        return 1;
    }

    char filename_png[60];
    sprintf(filename_png, "mandelbrot_degree_%.5f_%.5f.png", degree, frequency);

    

    // Внутри цикла по x и y:
    // double aspect_ratio = (double)width / height;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            
            // Внутри цикла по x и y
            if (degree == 2.0) {
                offsetX = -0.75;
                zoom = 3.0;
            }
            else if (degree < 1.0) zoom = 10.0;
            // Центрирование и масштабирование
            double cx = offsetX + (x / (double)width - 0.5) * zoom;
            double cy = offsetY + (y / (double)height - 0.5) * zoom;

            double zx = cx;
            double zy = cy;
            int iter = 1;

            // --- УНИВЕРСАЛЬНЫЙ ЦИКЛ ДЛЯ ЛЮБОЙ СТЕПЕНИ ---
            while (zx*zx + zy*zy <= 4.0 && iter < max_iter) {
                double r = sqrt(zx*zx + zy*zy);
                double theta = atan2(zy, zx);
                
                /*
                Формула: z = z^n + c
                𝐱_𝐧𝐞𝐱𝐭=𝐫^𝐧⋅𝐜𝐨𝐬⁡(𝐧⋅𝛉)+𝐱_𝐢𝐧𝐢𝐭
                𝐲_𝐧𝐞𝐱𝐭=𝐫^𝐧⋅𝐬𝐢𝐧(𝐧⋅𝛉)+𝐲_𝐢𝐧𝐢𝐭
                */
                double r_pow = pow(r, degree);
                double pow_theta = degree * theta;
                
                zx = r_pow * cos(pow_theta) + cx;
                zy = r_pow * sin(pow_theta) + cy;
                
                iter++;
            }
            
            // Получаем цвет
            Color pixel_color = get_color(iter, max_iter, zx, zy, degree, frequency, phase_r, phase_g, phase_b);
            
            // Запись в массив для PNG
            int index = (y * width + x) * channels;
            image_data[index]     = pixel_color.r;
            image_data[index + 1] = pixel_color.g;
            image_data[index + 2] = pixel_color.b;
            
        }
    }

    // Сохранение массива в PNG
    if (stbi_write_png(filename_png, width, height, channels, image_data, width * channels)) {
        printf("Фрактал успешно сохранен в %s\n", filename_png);
    } else {
        printf("Ошибка при сохранении PNG\n");
    }

    free(image_data);

// 2. Фиксируем время окончания
    time_t end_time = time(NULL);
    printf("Окончание: %s\n", ctime(&end_time));

    // 3. Вычисляем разницу (длительность)
    double diff = difftime(end_time, start_time);
    printf("Программа работала %.0f сек.\n", diff);

    return 0;
}
