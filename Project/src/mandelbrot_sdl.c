/*
Для реализации интерактива используем библиотеку SDL2.
Это позволит нам перемещаться по фракталу стрелками и масштабировать его клавишами + и - в реальном времени.
*/

#include <SDL2/SDL.h>
#include <omp.h>
#include <stdio.h>
#include <time.h>

void render(SDL_Renderer *renderer, SDL_Texture *texture, uint32_t *pixels, double zoom, double offsetX, double offsetY, int W, int H)
{
    // Логарифмические константы - используется для оптимизации log(log(sqrt(z)))
    const double inv_log2 = 1.0 / log(2.0);
    const double log_05 = log(0.5);

    #pragma omp parallel for schedule(dynamic) // Многопоточность через библиотеку OMP
    for (int y = 0; y < H; y++)
    {
        for (int x = 0; x < W; x++)
        {
            double cx = offsetX + (x - W / 2.0) * (zoom / W);
            double cy = offsetY + (y - H / 2.0) * (zoom / W);
            double zx =0, zy = 0;
            int iter = 0, max_iter = 128;

            while (zx*zx + zy*zy <= 4 && iter < max_iter)
            {
                double next_zx = zx * zx - zy * zy + cx;
                zy = 2 * zx * zy + cy;
                zx = next_zx;

                iter++;
            }

            uint32_t color = 0; // Инициализируем черным цветом
            if (iter < max_iter)
            {
                /*
                Вместо log(log(sqrt(zx² + zy²))) можно использовать свойства логарифмов, чтобы убрать корень:
                sqrt(z) = z^0.5
                log(z^0.5) = 0.5 * log(z)
                log(0.5 * log(z)) = log(0.5) + log(log(z))
                */
                double r2 = zx * zx + zy * zy; // Квадрат модуля
                // Вместо log(log(sqrt(r2))) / log(2)
                double mu = iter + 1 - ((log_05 + log(log(r2))) * inv_log2);

                // Вычисляем компоненты напрямую
                uint32_t r = (uint8_t)(sin(0.1 * mu + 255.0) * 127 + 128);
                uint32_t g = (uint8_t)(sin(0.1 * mu + 150.0) * 127 + 128);
                uint32_t b = (uint8_t)(sin(0.1 * mu + 0.0) * 127 + 128);

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
    // 1. Фиксируем время начала
    time_t start_time = time(NULL);
    printf("Начало: %s\n", ctime(&start_time));
    fflush(stdout);

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

    double zoom = 3.0, offsetX = -0.7, offsetY = 0;
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
                case SDLK_ESCAPE: running = 0; break;
                case SDLK_UP: offsetY -= 0.1 * zoom; break;
                case SDLK_DOWN: offsetY += 0.1 * zoom; break;
                case SDLK_LEFT: offsetX -= 0.1 * zoom; break;
                case SDLK_RIGHT: offsetX += 0.1 * zoom; break;
                case SDLK_EQUALS: zoom *= 0.8; break; // Клавиша +
                case SDLK_MINUS: zoom *= 1.2; break; // Клавиша -
                }
            }
        }
        render(ren, tex, pixels, zoom, offsetX, offsetY, W, H);
    }

    free(pixels);
    SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();

    // 2. Фиксируем время окончания
    time_t end_time = time(NULL);
    printf("\nОкончание: %s\n", ctime(&end_time));

    // 3. Вычисляем разницу (длительность)
    double diff = difftime(end_time, start_time);
    printf("Программа работала %.0f сек.\n", diff);

    return 0;
}
