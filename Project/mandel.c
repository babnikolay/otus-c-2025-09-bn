#include <stdio.h>

int main() {
    // Настройки изображения
    const int width = 800, height = 800;
    const int max_iter = 1000;
    FILE *fp = fopen("mandelbrot.ppm", "wb");

    // Заголовок PPM: формат P6 (бинарный), ширина, высота, макс. значение цвета
    fprintf(fp, "P6\n%d %d\n255\n", width, height);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Масштабируем координаты x и y в диапазон комплексной плоскости
            // x: от -2.0 до 0.5, y: от -1.25 до 1.25
            double ca = -2.0 + (x / (double)width) * 2.5;
            double cb = -1.25 + (y / (double)height) * 2.5;

            double a = 0, b = 0; // Начальное z = 0 (a + bi)
            int iter = 0;

            while (a*a + b*b <= 4 && iter < max_iter) {
                double next_a = a*a - b*b + ca;
                b = 2*a*b + cb;
                a = next_a;
                iter++;
            }

            // Простая раскраска: если точка внутри (iter == max_iter) — черная,
            // если снаружи — градиент серого
            unsigned char color = (iter == max_iter) ? 0 : (iter % 255);
            
            // Записываем RGB компоненты
            fputc(color, fp); // R
            fputc(color, fp); // G
            fputc(color, fp); // B
        }
    }

    fclose(fp);
    printf("Готово! Файл mandelbrot.ppm создан.\n");
    return 0;
}
