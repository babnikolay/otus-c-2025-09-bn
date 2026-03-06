#include "mandel.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

void help() {
    printf("usage: width height\n");
    printf("width   - Ширина окна\n");
    printf("height  - Высота окна\n");

}

// Вычисления цвета вынесены для универсальности
uint32_t compute_pixel(int x, int y, int W, int H, double zoom, double cx, double cy, int iter_count)
{
    const double inv_log2 = 1.0 / log(2.0);
    const double log_05 = log(0.5);

    double ca = cx + (x - W / 2.5) * (zoom / W);
    double cb = cy + (y - H / 2.5) * (zoom / W);
    double a = 0, b = 0, a2 = 0, b2 = 0;
    int iter = 0;

    while (a2 + b2 <= 4 && iter < iter_count)
    {
        b = 2 * a * b + cb;
        a = a2 - b2 + ca;
        a2 = a * a;
        b2 = b * b;
        iter++;
    }

    if (iter == iter_count)
        return 0xFF000000;

    double r2 = a2 + b2;
    double mu = iter + 1 - ((log_05 + log(log(r2))) * inv_log2);
    uint8_t r = (uint8_t)(sin(0.1 * mu + 0.0) * 127 + 128);
    uint8_t g = (uint8_t)(sin(0.1 * mu + 2.0) * 127 + 128);
    uint8_t bl = (uint8_t)(sin(0.1 * mu + 4.0) * 127 + 128);

    return (0xFF000000) | (r << 16) | (g << 8) | bl;
}

void save_png_with_dpi(double zoom, double cx, double cy, int iter)
{
    int w, h, dpi;
    printf("\n--- ПАРАМЕТРЫ СОХРАНЕНИЯ ---\n");
    printf("Введите ширину и высоту через пробел в пикселях (напр., 1920 1080): ");
    if (scanf("%d %d", &w, &h) != 2)
    {
        printf("Ошибка ввода. Введено недостаточно параметров.\n");
        return;
    }

    printf("Введите разрешение в пикселях на дюйм (dpi, напр., 300): ");
    if (scanf("%d", &dpi) != 1)
    {
        printf("Ошибка ввода. Не введено разрешение (dpi).\n");
        return;
    }

    uint32_t *img = malloc(w * h * sizeof(uint32_t));
    if (!img)
        return;

    printf("Рендеринг %dx%d... ", w, h);
    fflush(stdout);

#pragma omp parallel for schedule(dynamic)
    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            img[y * w + x] = compute_pixel(x, y, w, h, zoom, cx, cy, iter);
        }
    }

    // Если функция stbi_write_set_pixel_midpoint_resolution выдает ошибку,
    // значит ваша версия stb_image_write устарела.
    // Самый простой способ задать DPI в PNG без обновления библиотеки -
    // это оставить стандартный вызов, так как DPI в PNG лишь метаданные.

    char fname[256];
    snprintf(fname, sizeof(fname), "mandelbrot_%dx%d_%ddpi.png", w, h, dpi);

    // Записываем файл
    if (stbi_write_png(fname, w, h, 4, img, w * 4))
    {
        printf("\nУспешно сохранено: %s\n", fname);
    }
    else
    {
        printf("\nОшибка при сохранении!\n");
    }

    free(img);
}

// Обычный рендер для окна
void render(SDL_Renderer *ren, SDL_Texture *tex, uint32_t *pix, double zoom, ...)
{
    double cx = -2.0, cy = -1.25;
    int iter = 100, W = 800, H = 600;
    va_list args;
    va_start(args, zoom);
    RenderParam p;
    while ((p = va_arg(args, RenderParam)) != RENDER_END)
    {
        if (p == RENDER_CX)
            cx = va_arg(args, double);
        else if (p == RENDER_CY)
            cy = va_arg(args, double);
        else if (p == RENDER_ITER)
            iter = va_arg(args, int);
        else if (p == RENDER_WIDTH)
            W = va_arg(args, int);
        else if (p == RENDER_HEIGHT)
            H = va_arg(args, int);
    }
    va_end(args);

#pragma omp parallel for schedule(dynamic)
    for (int y = 0; y < H; y++)
        for (int x = 0; x < W; x++)
            pix[y * W + x] = compute_pixel(x, y, W, H, zoom, cx, cy, iter);

    SDL_UpdateTexture(tex, NULL, pix, W * 4);
    SDL_RenderCopy(ren, tex, NULL, NULL);
    SDL_RenderPresent(ren);
}
