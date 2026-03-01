/*
Для реализации интерактива используем библиотеку SDL2.
Это позволит нам перемещаться по фракталу стрелками и масштабировать его клавишами + и - в реальном времени.
*/

#include <SDL2/SDL.h>
#include <omp.h>
#include <stdio.h>

void render(SDL_Renderer *renderer, SDL_Texture *texture, uint32_t *pixels, double zoom, double cx, double cy, int W, int H)
{
    // Логарифмические константы - используется для оптимизации log(log(sqrt(z)))
    const double inv_log2 = 1.0 / log(2.0);
    const double log_05 = log(0.5);

#pragma omp parallel for schedule(dynamic) // Многопоточность через библиотеку OMP
    for (int y = 0; y < H; y++)
    {
        for (int x = 0; x < W; x++)
        {
            double ca = cx + (x - W / 2.0) * (zoom / W);
            double cb = cy + (y - H / 2.0) * (zoom / W);
            double a = 0, b = 0, a2 = 0, b2 = 0;
            int iter = 0, max_iter = 128;

            while (a2 + b2 <= 4 && iter < max_iter)
            {
                b = 2 * a * b + cb;
                a = a2 - b2 + ca;
                a2 = a * a;
                b2 = b * b;
                iter++;
            }

            uint32_t color = 0; // Инициализируем черным цветом
            if (iter < max_iter)
            {
                /*
                Вместо log(log(sqrt(a² + b²))) можно использовать свойства логарифмов, чтобы убрать корень:
                sqrt(z) = z^0.5
                log(z^0.5) = 0.5 * log(z)
                log(0.5 * log(z)) = log(0.5) + log(log(z))
                */
                double r2 = a * a + b * b; // Квадрат модуля
                // Вместо log(log(sqrt(r2))) / log(2)
                double mu = iter + 1 - ((log_05 + log(log(r2))) * inv_log2);

                // Вычисляем компоненты напрямую
                uint32_t r = (uint8_t)(sin(0.1 * mu + 0.0) * 127 + 128);
                uint32_t g = (uint8_t)(sin(0.1 * mu + 2.0) * 127 + 128);
                uint32_t b = (uint8_t)(sin(0.1 * mu + 4.0) * 127 + 128);

                // Собираем в формат 0xRRGGBB
                color = (r << 16) | (g << 8) | b;
            }
            pixels[y * W + x] = color;
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

    while (running)
    {
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
                running = 0;
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
                case SDLK_EQUALS:
                    zoom *= 0.8;
                    break; // Клавиша +
                case SDLK_MINUS:
                    zoom *= 1.2;
                    break; // Клавиша -
                }
            }
        }
        render(ren, tex, pixels, zoom, cx, cy, W, H);
    }

    free(pixels);
    SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
