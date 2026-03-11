#include <stdio.h>
#include <math.h>

int main()
{
    const int width = 1000, height = 1000;
    const int max_iter = 512; // Для цвета лучше брать не слишком большое число
    FILE *fp = fopen("mandelbrot_color.ppm", "wb");

    fprintf(fp, "P6\n%d %d\n255\n", width, height);

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            double ca = -2.0 + (x / (double)width) * 2.5;
            double cb = -1.25 + (y / (double)height) * 2.5;

            double a = 0, b = 0;
            int iter = 0;

            // Вычисление следующих значений комплексного числа z по формуле: z(n+1) = z(n)^2 + c
            while (a * a + b * b <= 4 && iter < max_iter)
            {
                double next_a = a * a - b * b + ca;
                b = 2 * a * b + cb;
                a = next_a;
                iter++;
            }

            unsigned char r, g, b_col;

            if (iter == max_iter)
            {
                // Точки внутри множества всегда черные
                r = g = b_col = 0;
            }
            else
            {
                // Коэффициент для плавности (от 0 до 1)
                double t = (double)iter / max_iter;

                /*
                Математическая раскраска через цвета задаётся через полиномиальные выражения вида:
                P(t) = C * (1−t)^a * t^b, где a + b = 4 (степень 4), а C — константа-множитель для настройки яркости,
                которые создают плавные градиенты с помощью полиномиальных функций, близких к базису Бернштейна.
                Множители (9, 15, 8.5) можно менять для поиска других оттенков
                */
                r = (unsigned char)(9.0 * (1 - t) * t * t * t * 255);
                g = (unsigned char)(15.0 * (1 - t) * (1 - t) * t * t * 255);
                b_col = (unsigned char)(8.5 * (1 - t) * (1 - t) * (1 - t) * t * 255);
            }

            fputc(r, fp);
            fputc(g, fp);
            fputc(b_col, fp);
        }
    }

    fclose(fp);
    printf("Цветное изображение создано!\n");

    return 0;
}
