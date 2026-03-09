#include <stdio.h>
#include <time.h>
#include <ctype.h>
#include <string.h>
#include "mandel.h"

#define BOLD_BLUE "\033[94m"
#define RESET      "\033[0m"

int main() {
    // 1. Фиксируем время начала
    time_t start_time = time(NULL);
    printf("Начало: %s\n", ctime(&start_time));
    fflush(stdout);

    double zoom = 2.5, cx = -1.0, cy = -0.3;
    int W = 800, H = 800, max_iter = 1000;
    double R = 0.8, G = 0.5, B = 2.1;

    int running = 1;
    SDL_Event event;

    char input[10];

    while (1) {
        printf("Сейчас применены параметры по умолчанию.\n");
        printf("Для подробности нажмите h.\n");
        printf("Хотите изменить параметры? (yY/"BOLD_BLUE"[nN]"RESET"/hH): ");
        if (fgets(input, sizeof(input), stdin) == NULL) break;
        input[strcspn(input, "\n")] = 0;

        // Сравнивает, игнорируя регистр (HELP == help == HeLp)
        if (strcasecmp(input, "h") == 0) {
            help();

            // 2. Фиксируем время окончания
            time_t end_time = time(NULL);
            printf("\nОкончание: %s\n", ctime(&end_time));

            // 3. Вычисляем разницу (длительность)
            double diff = difftime(end_time, start_time);
            printf("Программа работала %.0f сек.\n", diff);

            return 0;
        } else if (strcasecmp(input, "y") == 0) {
            get_input("Ширина окна - width: ", &W, "int", "800");
            get_input("Высота окна - height: ", &H, "int", "800");
            get_input("Масштабирование - zoom: ", &zoom, "double", "2.5");
            get_input("Максимальное количество итераций - MAX_ITER: ", &max_iter, "int", "1000");
            get_input("Красный для палитры: ", &R, "double", "0.8");
            get_input("Зелёный для палитры: ", &G, "double", "0.5");
            get_input("Синий для палитры: ", &B, "double", "2.1");

            printf("\nДанные приняты:\nW: %d, H: %d, Zoom: %.2f, MAX_ITER: %d\n", W, H, zoom, max_iter);
            printf("R: %.2f, G: %.2f, B: %.2f\n", R, G, B);

            break;
        } else if (input[0] == '\0' || strcasecmp(input, "n") == 0) {
            printf("Применены параметры по умолчанию!!!\n");
            printf("\nДанные по умолчанию: W: %d, H: %d, Zoom: %.2f, MAX_ITER: %d\n", W, H, zoom, max_iter);
            printf("R: %.2f, G: %.2f, B: %.2f\n", R, G, B);
            break;
        } else {
            printf("Неизвестная команда. Повторите ввод.\n");
        }
    }

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *win = SDL_CreateWindow("Mandelbrot [S - Save]", 100, 100, W, H, 0);
    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture *tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, W, H);
    uint32_t *pixels = malloc(W * H * 4);

    while (running) {
        /*
        Базовое количество итераций (например, 100) + добавка за зум
        Чем меньше zoom, тем больше итераций. Формула: 150 + 40 * |log10(zoom)|
        */
        int cur_iter = 150 + (int)(40.0 * fabs(log10(zoom)));
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                running = 0;
            
            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_s: // Нажать S для сохранения
                        save_png_with_dpi(zoom, cx, cy, cur_iter, R, G, B);
                        break;
                    case SDLK_ESCAPE: running = 0; break;
                    case SDLK_UP: cy -= 0.1 * zoom; break;
                    case SDLK_DOWN: cy += 0.1 * zoom; break;
                    case SDLK_LEFT: cx -= 0.1 * zoom; break;
                    case SDLK_RIGHT: cx += 0.1 * zoom; break;
                    case SDLK_EQUALS: zoom *= 0.8; break;
                    case SDLK_MINUS: zoom *= 1.2; break;
                }
            }
            
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                double m_ca = cx + (event.button.x - W / 2.5) * (zoom / W);
                double m_cb = cy + (event.button.y - H / 2.5) * (zoom / W);
                cx = m_ca;
                cy = m_cb;
                if (event.button.button == SDL_BUTTON_LEFT)
                    zoom *= 0.9;
                else
                    zoom /= 0.9;
            }
        }
        
        RENDER_SAFE(ren, tex, pixels, 
                    RENDER_ZOOM, zoom, 
                    RENDER_CX, cx, 
                    RENDER_CY, cy, 
                    RENDER_ITER, cur_iter, 
                    RENDER_WIDTH, W, 
                    RENDER_HEIGHT, H,
                    RENDER_R, R,
                    RENDER_G, G,
                    RENDER_B, B);

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
