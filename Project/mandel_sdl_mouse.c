/*

*/

/*
Для реализации интерактива используем библиотеку SDL2.
Это позволит нам перемещаться по фракталу стрелками и масштабировать его клавишами + и - в реальном времени.
*/

#include <SDL2/SDL.h>
#include <omp.h>
#include <stdio.h>

// Обновленная функция рендера теперь принимает iter_count извне
void render(SDL_Renderer *renderer, SDL_Texture *texture, uint32_t *pixels,
            double zoom, double cx, double cy, int iter_count, int W, int H)
{
#pragma omp parallel for schedule(dynamic)
    for (int y = 0; y < H; y++)
    {
        for (int x = 0; x < W; x++)
        {
            double ca = cx + (x - W / 2.0) * (zoom / W);
            double cb = cy + (y - H / 2.0) * (zoom / W);
            double a = 0, b = 0, a2 = 0, b2 = 0;
            int iter = 0;

            // Используем переданное значение iter_count
            while (a2 + b2 <= 4 && iter < iter_count)
            {
                b = 2 * a * b + cb;
                a = a2 - b2 + ca;
                a2 = a * a;
                b2 = b * b;
                iter++;
            }

            uint32_t color_val;
            if (iter == iter_count)
            {
                color_val = 0; // Внутри множества — черный
            }
            else
            {
                // Плавная раскраска (упрощенная)
                double mu = iter + 1 - log(log(sqrt(a2 + b2))) / log(2.0);
                uint8_t r = (uint8_t)(sin(0.1 * mu + 0.0) * 127 + 128);
                uint8_t g = (uint8_t)(sin(0.1 * mu + 2.0) * 127 + 128);
                uint8_t bl = (uint8_t)(sin(0.1 * mu + 4.0) * 127 + 128);
                color_val = (r << 16) | (g << 8) | bl;
            }
            pixels[y * W + x] = color_val;
        }
    }
    SDL_UpdateTexture(texture, NULL, pixels, W * sizeof(uint32_t));
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        fprintf(stderr, "usage: %s width height\n", argv[0]);
        printf("width   - Ширина окна\n");
        printf("height  - Высота окна\n");
        return 1;
    }

    int W = atoi(argv[1]), H = atoi(argv[2]);
    printf("Ширина окна %u\n", W);
    printf("Высота окна %u\n", H);

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *win = SDL_CreateWindow("Mandelbrot Explorer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, W, H, 0);
    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture *tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, W, H);
    uint32_t *pixels = malloc(W * H * sizeof(uint32_t));

    double zoom = 1.5, cx = -0.5, cy = 0;
    int running = 1;
    SDL_Event e;
    int max_iter = 128;

    while (running)
    {
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
                running = 0;

            // Приближение мышкой
            if (e.type == SDL_MOUSEBUTTONDOWN)
            {
                double m_ca = cx + (e.button.x - W / 2.0) * (zoom / W);
                double m_cb = cy + (e.button.y - H / 2.0) * (zoom / W);
                cx = m_ca;
                cy = m_cb;
                if (e.button.button == SDL_BUTTON_LEFT)
                    zoom *= 0.3;
                else if (e.button.button == SDL_BUTTON_RIGHT)
                    zoom /= 0.3;
            }

            // Управление клавиатурой
            if (e.type == SDL_KEYDOWN)
            {
                switch (e.key.keysym.sym)
                {
                case SDLK_ESCAPE:
                    running = 0;
                    break;
                case SDLK_UP:
                    cy -= 0.1 * zoom;
                    break;
                case SDLK_DOWN:
                    cy += 0.1 * zoom;
                    break;
                case SDLK_LEFT:
                    cx -= 0.1 * zoom;
                    break;
                case SDLK_RIGHT:
                    cx += 0.1 * zoom;
                    break;
                // Увеличиваем детализацию (итерации) клавишами [ и ]
                case SDLK_LEFTBRACKET:
                    max_iter -= 50;
                    break;
                case SDLK_RIGHTBRACKET:
                    max_iter += 50;
                    break;
                }
            }
        }

        // Базовое количество итераций (например, 100) + добавка за зум
        // Чем меньше zoom, тем больше итераций.
        // Формула: 150 + 40 * |log10(zoom)|
        int current_max_iter = 150 + (int)(40.0 * fabs(log10(zoom)));

        // Ограничим сверху для производительности
        if (current_max_iter > 5000)
            current_max_iter = 5000;

        render(ren, tex, pixels, zoom, cx, cy, current_max_iter, W, H);
    }

    free(pixels);
    SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
