#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>
#include <time.h>
#include <stdbool.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define WIDTH 2400
#define HEIGHT 2400
#define SAMPLES 600000000 // 100 млн точек. Для идеального фона без шума ставьте 300-500 млн.

// Лимиты итераций для разделения каналов
#define MAX_ITER 5000 
#define THRESHOLD_RED   1000    // Долгоживущие точки -> Красный
#define THRESHOLD_GREEN 100     // Средние -> Зеленый
#define THRESHOLD_BLUE  20      // Быстрые -> Синий (Фон)
#define MAX_ITER_DEEP 50000     // В 10 раз больше деталей

const double X_MIN = -2.0, X_MAX = 1.0;
const double Y_MIN = -1.5, Y_MAX = 1.5;

double *r_plane, *g_plane, *b_plane, *d_plane;

// Оптимизация: пропуск точек внутри множества Мандельброта
bool is_in_mandelbrot(double x, double y) {
    double y2 = y * y;
    double q = (x - 0.25) * (x - 0.25) + y2;
    if (q * (q + (x - 0.25)) < 0.25 * y2) return true;
    if ((x + 1.0) * (x + 1.0) + y2 < 0.0625) return true;
    return false;
}

// Функция отрисовки траектории в конкретный массив
void trace_to_plane(double cr, double ci, int age, double *plane) {
    double zr = 0, zi = 0;
    for (int i = 0; i < age; i++) {
        double zr2 = zr * zr, zi2 = zi * zi;
        double zi_next = 2.0 * zr * zi + ci;
        zr = zr2 - zi2 + cr;
        zi = zi_next;

        // Поворот на 90 градусов (вертикальный вид)
        int px = (int)((zi - Y_MIN) / (Y_MAX - Y_MIN) * WIDTH);
        int py = (int)((zr - X_MIN) / (X_MAX - X_MIN) * HEIGHT);

        if (px >= 0 && px < WIDTH && py >= 0 && py < HEIGHT) {
            #pragma omp atomic
            plane[py * WIDTH + px]++;
        }
    }
}
int main(int argc, char **argv) {
    // Фиксируем время начала
    time_t start_time = time(NULL);
    printf("Начало: %s\n", ctime(&start_time));

    if (argc != 4) {
        printf("Использование: %s <k_r> <k_g> <k_b> <k_d>\n", argv[0]);
        printf("Пример: %s 0.8 0.7 0.5 0.6\n", argv[0]);

        // Фиксируем время окончания
        time_t end_time = time(NULL);
        printf("Окончание: %s\n", ctime(&end_time));

        // Вычисляем разницу (длительность)
        double diff = difftime(end_time, start_time);
        printf("Программа работала %.0f сек.\n", diff);

        return 1;
    }

    double k_r = atof(argv[1]);
    double k_g = atof(argv[2]);
    double k_b = atof(argv[3]);
    printf("Коэффициент индивидуальной гаммы для сочности k_r = %.3f\n", k_r);
    printf("Коэффициент индивидуальной гаммы для сочности k_g = %.3f\n", k_g);
    printf("Коэффициент индивидуальной гаммы для сочности k_b = %.3f\n", k_b);

    r_plane = (double*)calloc(WIDTH * HEIGHT, sizeof(double));
    g_plane = (double*)calloc(WIDTH * HEIGHT, sizeof(double));
    b_plane = (double*)calloc(WIDTH * HEIGHT, sizeof(double));

    long long progress = 0;
    printf("\nГенерация радужного Будды на %d потоках...\n", omp_get_max_threads());

    #pragma omp parallel
    {
        unsigned int seed = time(NULL) ^ omp_get_thread_num();
        #pragma omp for schedule(dynamic, 5000)
        for (long i = 0; i < SAMPLES; i++) {
            if (i % 10000000 == 0) {
                #pragma omp atomic
                progress += 10000000;
                if (omp_get_thread_num() == 0) {
                    printf("\rПрогресс: %.2f%% ", (double)progress / SAMPLES * 100.0);
                    fflush(stdout);
                }
            }

            double cr = ((double)rand_r(&seed)/RAND_MAX) * (X_MAX-X_MIN) + X_MIN;
            double ci = ((double)rand_r(&seed)/RAND_MAX) * (Y_MAX-Y_MIN) + Y_MIN;

            if (is_in_mandelbrot(cr, ci)) continue;

            // Узнаем, когда точка убежит
            double zr = 0, zi = 0;
            int age = 0;
            while (zr*zr + zi*zi < 4.0 && age < MAX_ITER) {
                double zr_new = zr*zr - zi*zi + cr;
                zi = 2.0*zr*zi + ci;
                zr = zr_new;
                age++;
            }

            // Если точка убежала, рисуем её в ОДИН из каналов (изоляция цвета)
            if (age < MAX_ITER) {
                if (age > THRESHOLD_RED)        trace_to_plane(cr, ci, age, r_plane);
                else if (age > THRESHOLD_GREEN) trace_to_plane(cr, ci, age, g_plane);
                else if (age > THRESHOLD_BLUE)  trace_to_plane(cr, ci, age, b_plane);
            }
        }
    }

    double max_r = 0, max_g = 0, max_b = 0;
    for (int i = 0; i < WIDTH * HEIGHT; i++) {
        if (r_plane[i] > max_r) max_r = r_plane[i];
        if (g_plane[i] > max_g) max_g = g_plane[i];
        if (b_plane[i] > max_b) max_b = b_plane[i];
    }

    // Подготовка данных для PNG (3 байта на пиксель)
    unsigned char* image_data = (unsigned char*)malloc(WIDTH * HEIGHT * 3);

    for (int i = 0; i < WIDTH * HEIGHT; i++) {
        /*
        Логарифмическое масштабирование с индивидуальной гаммой для сочности
        r=0.8, g=0.7, b=0.5 - голубой фон с белым Буддой
        0.6, 0.75, 0.45 - розоватый Будда
        Нормализуем значения (v_r, v_g, v_b — это log-значения от 0.0 до 1.0)
        */
        double v_r = log1p(r_plane[i]) / log1p(max_r);
        double v_g = log1p(g_plane[i]) / log1p(max_g);
        double v_b = log1p(b_plane[i]) / log1p(max_b);

        double r = pow(v_r, k_r);
        double g = pow(v_g, k_g);
        double b = pow(v_b, k_b); // Низкая гамма для яркого фона

        // Contrast Stretch (делаем тени "шоколадными", а свет "горящим")
        r = (r > 0.1) ? pow(r, 1.1) : r * 0.8;

        // Смешиваем каналы для переливов (малиновый верх, бирюзовые бока)
        // int R = (int)((r * 1.0 + b * 0.1) * 255);
        // int G = (int)((g * 1.0 + r * 0.1) * 255);
        // int B = (int)((b * 1.0 + g * 0.2) * 255);

        // Смешивание каналов для ЗОЛОТА:
        // R = основа (красно-оранжевый)
        // G = придает желтизну (примерно 80-85% от R)
        // B = придает блеск и уводит желтый в "дорогой" лимонный, а не в рыжий
        int R = (int)((r * 1.0 + b * 0.1) * 255);
        int G = (int)((g * 0.85 + r * 0.1) * 255); // Основной золотой тон
        int B = (int)((b * 0.5 + g * 0.2) * 255);  // Синий подмешиваем чуть-чуть для ярко-желтых бликов

        // Ограничители (clamping)
        if (R > 255) R = 255;
        if (G > 255) G = 255;
        if (B > 255) B = 255;
        if (R > 140) B += (R - 140) * 0.5;

        image_data[i * 3 + 0] = R > 255 ? 255 : R;
        image_data[i * 3 + 1] = G > 255 ? 255 : G;
        image_data[i * 3 + 2] = B > 255 ? 255 : B;
    }

    stbi_write_png("buddhabrot.png", WIDTH, HEIGHT, 3, image_data, WIDTH * 3);

    printf("\nГотово! Результат в buddhabrot.png\n\n");
    free(r_plane); free(g_plane); free(b_plane);

    // 2. Фиксируем время окончания
    time_t end_time = time(NULL);
    printf("Окончание: %s\n", ctime(&end_time));

    // 3. Вычисляем разницу (длительность)
    double diff = difftime(end_time, start_time);
    printf("Программа работала %.0f сек.\n", diff);

    return 0;
}
